/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   pmu_publisher.cpp
**
**    @brief  Collect hardware telemetry from Intel platforms
**
**
********************************************************************************
*/

#include <memory>

#include <boost/dll.hpp>

#include "pipeline_manager.h"
#include "pmu_config_agent.h"
#include "pmu_publisher.h"
#include "lbr_config_agent.h"
#include "driver_interface.h"
#include "driver_interface_impl.h"
#include "data_source_own_thread.h"

using namespace tbb::flow;

namespace bit_shovel_plugins
{

    namespace internal
    {
        constexpr auto pmu_publisher_id = "pmu_publisher";
        constexpr auto pmu_publisher_description =
            "Gather hardware telemetry data from the PMU "
#if defined(__linux__)
            "using the built-in 'perf' kernel module. "
#else
            "using the custom TDT Driver. "
#endif
            "Requires a heuristic plugin to be loaded to know required configuration.";
    }  // namespace internal

    pmu_publisher::pmu_publisher(std::unique_ptr<bit_shovel::plugin_config_t> config)
        : bit_shovel::plugin_base(internal::pmu_publisher_id,
              internal::pmu_publisher_description,
              std::move(config))
        , m_driver_interface(std::make_shared<bit_shovel_plugins::driver_interface_impl>())
        , m_driver_config()
        , m_lbr_config_opts(std::make_shared<std::list<std::string>>())
        , m_send_event_slots(false)
        , m_send_lbr_opts(false)
    {}

    void pmu_publisher::register_types(bit_shovel::plugin_type_registry& registry)
    {
        // include all potential types
        registry.register_as_sink<hw_telemetry_process_list_t>();
        registry.register_as_source<hw_telemetry_records_t>();
        registry.register_as_source<hw_pmu_events_slots_t>();
        registry.register_as_source<hw_lbr_config_opts_t>();
    }

    bit_shovel::result_type pmu_publisher::init(const bit_shovel::channel_registry& registry,
        bit_shovel::data_network& network,
        bit_shovel::data_source_list_t& data_source_list_out)
    {
        bit_shovel::result_type result;  // success by default

        // in try-catch block as get_sources() and get_sinks can throw exceptions from
        // boost::any_cast
        try
        {
            // check dependencies
            if (registry.get_sinks<hw_telemetry_records_t>().empty())
            {
                result.add_failure() << "Dependency failure: unable to find required sink for "
                                        "hw_telemetry_records_t."
                                     << std::endl;
            }

            if (!registry.get_sinks<hw_pmu_events_slots_t>().empty())
            {
                m_send_event_slots = true;
            }

            if (!registry.get_sinks<hw_lbr_config_opts_t>().empty())
            {
                m_send_lbr_opts = true;
            }
        }
        catch (boost::bad_any_cast& e)
        {
            result.add_failure() << "pmu_publisher::init() " << e.what() << std::endl;
        }

        // try to initialize contact with the driver
        if (result && !m_driver_interface->init())
        {
            result.add_failure() << "Unable to initialize communication with the driver."
                                 << std::endl;
        }

        auto& config = *this->configuration();

        const bool bleed_prevention_enabled =
            config.get(this->id() + ".bleed_prevention_enabled", true);

        m_driver_config.bleed_prevention_enabled = bleed_prevention_enabled;

        // read event strings from config
        if (result)
        {
            // read the config!
            auto events_opt = config.get_child_optional(this->id() + ".event_strings");
            if (events_opt)
            {
                hw_pmu_events_config_t events_config;

                for (auto& event : events_opt.get())
                {  // TODO: force lower case?
                    events_config.push_back(event.second.data());
                }

                result =
                    pmu_config_agent::parse(events_config, m_driver_config, m_pmu_events_slots);
            }
            else
            {  // can't configure without strings!
                result.add_failure()
                    << "No event strings were found in the configuration." << std::endl;
            }
        }

        // read lbr settings from config
        if (result)
        {
            // read the config!
            auto lbr_enable = config.get_optional<bool>(this->id() + ".lbr_enable");

            // lbr_enable might not be specified for detecting certain threats e.g. sidechannel
            // if we found lbr_enable and it is on then check for rest of config
            if (lbr_enable && lbr_enable.value())
            {
                auto lbr_config_string =
                    config.get_optional<std::string>(this->id() + ".lbr_configuration");
                auto lbr_count = config.get_optional<uint32_t>(this->id() + ".num_lbrs_to_collect");

                if (!lbr_config_string)
                {
                    result.add_failure()
                        << "Required field lbr_configuration not found." << std::endl;
                }
                else if (!lbr_count)
                {
                    result.add_failure()
                        << "Required field num_lbrs_to_collect not found." << std::endl;
                }

                // okay we have all the required fields
                if (result)
                {
                    lbr_config_t lbr_config;

                    lbr_config.lbr_enable = true;
                    lbr_config.msr_lbr_select_config = lbr_config_string.value();
                    lbr_config.num_lbrs_to_collect = lbr_count.value();

                    result =
                        lbr_config_agent::parse(lbr_config, m_driver_config, m_lbr_config_opts);
                }
            }  // else lbr_enable was not specified or it was false
        }

        if (result)
        {
            // filtered process catching node
            auto filter_node =
                std::make_shared<tbb::flow::function_node<hw_telemetry_process_list_t>>(
                    network.graph(),
                    1,
                    [this, &network](const hw_telemetry_process_list_t& filter) {
                        // When the pipeline is terminated (kill signal) the
                        // filter list may intermittently be nullptr or empty.
                        if (filter == nullptr || filter->empty())
                        {
                            network.push<bit_shovel::pipeline_message_t>(
                                {bit_shovel::pipeline_message_type_t::abort,
                                    "Process Filter list is nullptr or Empty!!!"});
                        }
                        else
                        {
                            const size_t byte_size = filter->size() * sizeof(filter->at(0));
                            if (byte_size <= std::numeric_limits<uint32_t>::max())
                            {
                                m_driver_interface->set_process_filter(
                                    filter->data(), static_cast<uint32_t>(byte_size));
                            }
                            else
                            {
                                network.push<bit_shovel::pipeline_message_t>(
                                    {bit_shovel::pipeline_message_type_t::abort,
                                        "Cannot send process list more than uint32_t bytes long to "
                                        "the driver."});
                            }
                        }
                    });

            result = network.add_sink_node<hw_telemetry_process_list_t>(filter_node);
        }

        if (result)
        {
            // process data pushing source
            auto source =
                std::make_shared<bit_shovel::data_source_own_thread<hw_telemetry_records_t>>(
                    network,
                    // harvest func
                    [this](hw_telemetry_records_t& data) -> bool {
                        return m_driver_interface->read_telemetry_data(data);
                    },
                    // start func
                    [this]() -> bit_shovel::result_type {
                        bit_shovel::result_type result;  // success by default

                        if (!m_driver_interface->start(m_driver_config))
                        {
                            result.add_failure()
                                << "Attempt to start the driver data collection failed."
                                << std::endl;
                        }

                        return result;
                    },
                    // stop func
                    [this]() -> bit_shovel::result_type {
                        bit_shovel::result_type result;  // success by default
                        if (!m_driver_interface->stop())
                            result.add_failure()
                                << "Unable to successfully stop driver data collection."
                                << std::endl;
                        return result;
                    });

            data_source_list_out.push_back(source);
        }

        return result;
    }

    bit_shovel::result_type pmu_publisher::push_configs(bit_shovel::data_network& network)
    {
        bit_shovel::result_type result;  // success by default

        // send out the active pmu config to all plugins
        if (m_send_event_slots)
        {
            result = network.push<decltype(m_pmu_events_slots)>(m_pmu_events_slots);
        }

        if (result && m_send_lbr_opts)
        {
            // send out the active lbr config to all plugins
            result = network.push<decltype(m_lbr_config_opts)>(m_lbr_config_opts);
        }

        return result;
    }

    // Factory method
    std::shared_ptr<pmu_publisher> pmu_publisher::create(
        std::unique_ptr<bit_shovel::plugin_config_t> config)
    {
        return std::make_shared<bit_shovel_plugins::pmu_publisher>(std::move(config));
    }

#ifndef DLL_EXPORT
#    define DLL_EXPORT
    BOOST_DLL_ALIAS(
        bit_shovel_plugins::pmu_publisher::create,  // <-- this function is exported with...
        create_plugin)                              // <-- ...this alias name
#endif

}  // namespace bit_shovel_plugins
