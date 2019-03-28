/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   process_monitor.cpp
**
**    @brief  Read process information about the system for the graph
**
**
********************************************************************************
*/

#include <memory>
#include <string>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>

#include "process_monitor.h"
#include "process_monitor_types.h"
#include "detection_event_type.h"
#include "pmu_publisher_types.h"
#include "data_source_own_thread.h"
#include "os_name.h"

using namespace tbb::flow;
using namespace bit_shovel_plugins;

namespace bit_shovel_plugins
{
    namespace internal
    {
        constexpr auto process_monitor_id = "process_monitor";
        constexpr auto process_monitor_description =
            "Gathers process information from the OS. Provides process names to reporters and "
            "process "
            "filtering whitelist to drivers where applicable and enabled.";
    }  // namespace internal

    process_monitor::process_monitor(std::unique_ptr<bit_shovel::plugin_config_t> config)
        : bit_shovel::plugin_base(internal::process_monitor_id,
              internal::process_monitor_description,
              std::move(config))
        , m_sys_process(std::make_shared<system_process_impl>())
        , m_process_sampler_mutex()
        , m_processes()
        , m_proc_filter_list()
        , m_pid()
        , m_first_run(true)
        , m_unknown_process_name()
        , m_sampling_rate_in_ms()
    {}

    void process_monitor::register_types(bit_shovel::plugin_type_registry& registry)
    {
        // include all potential types
        registry.register_as_sink<detection_event_list_t>();
        registry.register_as_source<detection_event_with_process_name_list_t>();
        registry.register_as_source<hw_telemetry_process_list_t>();
    }

    std::string process_monitor::get_process_name(const pid_t pid)
    {
        static std::pair<pid_t, std::string> cache = {0, ""};
        std::string retVal = m_unknown_process_name;
        if (cache.first == pid)
        {
            retVal = cache.second;
        }
        else
        {
            std::lock_guard<std::recursive_mutex> lock(m_process_sampler_mutex);
            process_data_list_t::iterator it;
            it = m_processes.find(pid);
            if (it != m_processes.end())
            {
                retVal = m_processes[pid].name;
                cache.first = pid;
                cache.second = retVal;
            }
        }
        return retVal;
    }
    std::string process_monitor::get_process_path(const pid_t pid)
    {
        static std::pair<pid_t, std::string> cache = {0, ""};
        std::string retVal = "";
        if (cache.first == pid)
        {
            retVal = cache.second;
        }
        else
        {
            std::lock_guard<std::recursive_mutex> lock(m_process_sampler_mutex);
            process_data_list_t::iterator it;
            it = m_processes.find(pid);
            if (it != m_processes.end())
            {
                retVal = m_processes[pid].executable_path;
                cache.first = pid;
                cache.second = retVal;
            }
        }
        return retVal;
    }

    bool process_monitor::get_pids_by_path(const std::string& path, pid_list_t& pids)
    {
        const boost::filesystem::path filter_path = path;

        for (const auto& process : m_processes)
        {
            // If executable path is blank, then return it as part of the query
            if (process.second.executable_path.empty())
            {
                pids.insert(process.first);
            }
            else
            {
                boost::system::error_code ec = {};
                boost::filesystem::path executable_path = process.second.executable_path;

                // When the filter path is a directory strip the filename from the executable path
                // before comparison
                if (boost::filesystem::is_directory(filter_path))
                {
                    executable_path = executable_path.parent_path();
                }

                // See if both paths resolve to the same file system entity
                if (boost::filesystem::equivalent(filter_path, executable_path, ec))
                {
                    pids.insert(process.first);
                }
            }
        }
        return !m_processes.empty();
    }

    bit_shovel::result_type process_monitor::init(const bit_shovel::channel_registry& registry,
        bit_shovel::data_network& network,
        bit_shovel::data_source_list_t& data_source_list_out)
    {
        bit_shovel::result_type result;  // success by default

        auto& config = *this->configuration();

        /* read configuration values */
        m_unknown_process_name = config.get(this->id() + ".unknown_process_name", "Process");
        m_sampling_rate_in_ms = config.get(this->id() + ".sampling_rate_in_ms", 500u);
        bool enable_process_filter = config.get(this->id() + ".enable_process_filter", true);

        /* read the process filter strings from the configuration if needed */

        if (enable_process_filter)
        {
            // read the process filter
            auto filters_opt = config.get_child_optional(
                this->id() + ".process_filter." + OS_NAME);  // OS_NAME is from os_name.h

            // required configuration item (may be empty but must exist)
            if (!filters_opt.is_initialized())
            {
                result.add_failure() << "Process filtering for the process_monitor plugin is "
                                        "enabled but no filter list is in the config."
                                     << std::endl;
            }

            // in try-catch block as get_sources() and get_sinks can throw exceptions from
            // boost::any_cast
            try
            {
                if (result && registry.get_sinks<hw_telemetry_process_list_t>().empty())
                {
                    result.add_failure() << "Dependency failure: process filtering is enabled but "
                                            "there is no sink for hw_telemetry_process_list_t."
                                         << std::endl;
                }
            }
            catch (boost::bad_any_cast& e)
            {
                result.add_failure() << "process_monitor::init() " << e.what() << std::endl;
            }

            if (result)
            {
                for (auto& pair : filters_opt.get())
                {
                    std::string filter = pair.second.data();
                    m_proc_filter_list.push_back(filter);
                }
            }

            if (result && m_proc_filter_list.empty())
            {
                // if enabled, we shouldn't accept a blank list since that was probably a mistake
                // and would be hard to detect at first
                result.add_failure() << "Process filtering for the process_monitor plugin is "
                                        "enabled but filter list in the config is empty."
                                     << std::endl;
            }

            if (result)
            {
                /* data source to collect process information */
                m_pid = m_sys_process->current_process_id();

                // process data pushing source
                auto source = std::make_shared<
                    bit_shovel::data_source_own_thread<hw_telemetry_process_list_t>>(network,
                    // harvest func
                    [this](hw_telemetry_process_list_t& pid_list) -> bool {
                        if (m_first_run)
                        {
                            m_first_run = false;
                        }
                        else
                        {
                            std::this_thread::sleep_for(
                                std::chrono::milliseconds(m_sampling_rate_in_ms));
                        }

                        {
                            std::lock_guard<std::recursive_mutex> lock(m_process_sampler_mutex);
                            m_processes.clear();

                            if (m_sys_process->populate_process_list(m_processes))
                            {

                                auto pid_filter_list = pid_list_t();

                                for (auto& path : m_proc_filter_list)
                                {
                                    get_pids_by_path(path, pid_filter_list);
                                }

                                pid_filter_list.insert(m_pid);  // Add the PID for this process
                                pid_list =
                                    std::make_shared<hw_telemetry_process_list_t::element_type>(
                                        pid_filter_list.begin(), pid_filter_list.end());
                            }
                        }

                        // never stop trying to collect new data
                        return true;
                    });

                data_source_list_out.push_back(source);
            }
        }

        /* node to add process names to detection reports */
        if (result)
        {

            auto node = std::make_shared<
                function_node<detection_event_list_t, detection_event_with_process_name_list_t>>(
                network.graph(),
                1,
                [this](const detection_event_list_t& events)
                    -> detection_event_with_process_name_list_t {
                    detection_event_with_process_name_list_t dex;
                    for (const auto& it : events)
                    {
                        dex.push_back(detection_event_with_process_name_t(
                            it, get_process_name(it.process_id), get_process_path(it.process_id)));
                    }

                    return dex;
                });

            result = network.add_source_node<detection_event_with_process_name_list_t>(node);
            if (result) result = network.add_sink_node<detection_event_list_t>(node);
        }

        return result;
    }

    std::shared_ptr<process_monitor> process_monitor::create(
        std::unique_ptr<bit_shovel::plugin_config_t> config)
    {
        return std::make_shared<bit_shovel_plugins::process_monitor>(std::move(config));
    }

#ifndef DLL_EXPORT
#    define DLL_EXPORT
    BOOST_DLL_ALIAS(
        bit_shovel_plugins::process_monitor::create,  // <-- this function is exported with...
        create_plugin)                                // <-- ...this alias name
#endif

}  // namespace bit_shovel_plugins
