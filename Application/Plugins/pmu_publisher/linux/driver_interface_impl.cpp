/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   driver_interface_impl.cpp
**
**    @brief  User space driver communication implementation for the Linux kernel 'perf' module.
**
**
********************************************************************************
*/
#ifdef DEBUG
#    include <iostream>
#endif

#include <cstring>
#include <memory>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/utsname.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <mutex>

#include "driver_interface_impl.h"

namespace bit_shovel_plugins
{

    driver_interface_impl::driver_interface_impl()
        : m_num_cpus(sysconf(_SC_NPROCESSORS_ONLN))
        , m_is_running(false)
        , m_group_events()
        , m_epollfd(internal::IO_ERROR)
        , m_filter_mutex()
    {}

    driver_interface_impl::~driver_interface_impl()
    {
        m_is_running = false;
        stop();
        m_group_events.clear();
    }

    bool driver_interface_impl::init()
    {
        bool success = false;

        // The existence of the perf_event_paranoid file is the official method for
        // determining if a kernel supports perf_event_open().
        const boost::filesystem::path PERF_PROC_FILE("/proc/sys/kernel/perf_event_paranoid");
        if (boost::filesystem::exists(PERF_PROC_FILE))
        {
            // Create group event container for each CPU
            m_group_events.resize(m_num_cpus);
            for (auto cpu = 0u; cpu < m_num_cpus; ++cpu)
            {
                m_group_events[cpu].cpu(cpu);
            }

            success = true;
        }

        return success;
    }

    bool driver_interface_impl::start(const PMU_CONFIGURATION_NODE& config)
    {
        bool success = false;
        // TODO: Implement Bleed prevention
#ifdef DEBUG
        if (config.bleed_prevention_enabled)
        {
            std::cout << "WARNING: Bleed prevention is not supported on Linux." << std::endl;
        }
#endif
        if (!m_is_running)
        {
            success = _configure_events(config);
            if (success)
            {
                success = _start_data_collection();
            }
        }
        return success;
    }

    bool driver_interface_impl::stop()
    {
        m_is_running = false;
        if (m_epollfd != internal::IO_ERROR)
        {
            close(m_epollfd);
            m_epollfd = internal::IO_ERROR;
        }
        for (const auto& group : m_group_events)
        {
            group.disable();
        }
        return true;
    }

    bool driver_interface_impl::read_driver_version(TD_VERSION_NODE& version)
    {
        // Have not found any way to determine the 'perf' version. So get the kernel release.
        version = {0};
        utsname sys_info = {0};
        const int status = uname(&sys_info);
        if (status != internal::IO_ERROR)
        {
            constexpr auto EXPECTED_TOKEN_COUNT = 4;
#ifdef DEBUG
            std::cout << "UTS release : " << sys_info.release << std::endl;
#endif
            std::vector<std::string> sys_release_tokens;
            boost::algorithm::split(
                sys_release_tokens, sys_info.release, boost::algorithm::is_punct());
            if (!sys_release_tokens.empty() && (sys_release_tokens.size() >= EXPECTED_TOKEN_COUNT))
            {
                version.u1.s1.major = std::atoi(sys_release_tokens[0].c_str());
                version.u1.s1.minor = std::atoi(sys_release_tokens[1].c_str());
                version.u1.s1.api = std::atoi(sys_release_tokens[2].c_str());
                version.u1.s1.update = std::atoi(sys_release_tokens[3].c_str());
            }
        }
        return true;
    }

    bool driver_interface_impl::read_telemetry_data(hw_telemetry_records_t& data)
    {
        bool ret_val = false;
        if (m_is_running)
        {
            do
            {
                ret_val = _collect_telemetry_data(data);
            } while (m_is_running && !ret_val);
        }
        return ret_val;
    }

    bool driver_interface_impl::set_process_filter(uint32_t* buffer, const uint32_t buffer_size)
    {
        bool success = false;
        if (buffer_size > sizeof(uint32_t))
        {
            const size_t pid_count = buffer_size / sizeof(uint32_t);
            std::lock_guard<std::mutex> lock(m_filter_mutex);

            size_t i = 0;
            while (i < pid_count)
            {
                m_process_filter_set.insert(buffer[i++]);
            }
            success = true;
        }
        return success;
    }

    bool driver_interface_impl::_is_pid_filtered(const pid_t pid)
    {
        std::lock_guard<std::mutex> lock(m_filter_mutex);

        const auto count = m_process_filter_set.count(pid);

        return (count != 0);
    }

    bool driver_interface_impl::_start_data_collection()
    {
        bool success = false;
        // For each CPU
        for (auto& group : m_group_events)
        {
            success = group.open();
            if (success)
            {
                success = group.enable();
            }
            if (!success)
            {
                break;
            }
        }
        if (success)
        {
            success = _init_polling();
            m_is_running = success;
        }
        return success;
    }

    bool driver_interface_impl::_configure_events(const PMU_CONFIGURATION_NODE& pmu_config)
    {
        bool success = false;

        // temporary storage for the list of events until they are copied to the CPU event groups.
        std::vector<pmu_event> event_list;

        // NOTE: The data extraction from the perf sample record is dependent on the order of the
        // events in the list. If changed here it must also be changed in
        // 'event_group::_convert_to_telemetry_records()'

        // NOTE: The first event in the list is the group leader and must have a sample period
        // (Interrupt rate).

        //
        // Create the Programmable events
        pmu_event event(PERF_TYPE_RAW);

        // Create PMU events for non-zero eventSel values
        for (auto i = 0; i < MAX_APP_PROG_EVENTS; ++i)
        {
            if (pmu_config.eventSel[i] != 0)
            {
                event.configure(pmu_config.eventSel[i]);
                event.sample_period(pmu_config.samplesPerInterrupt[i]);
                event_list.push_back(event);
            }

            // If none of the events have a sampling rate no data will be recorded,
            // false will be returned!
            if (pmu_config.samplesPerInterrupt[i] != 0)
            {
                success = true;
            }
        }

        if (success)
        {
            // Create the Fixed events. (Instructions, Cycles, Ref Cycles)
            pmu_event fixed_event(PERF_TYPE_HARDWARE);

            fixed_event.configure(PERF_COUNT_HW_INSTRUCTIONS);
            event_list.push_back(fixed_event);

            // The PERF_COUNT_HW_CPU_CYCLES uses a programmable event
            // which prevents 4 programmable counters from collecting
            // data! (i.e. CJ profiles use 4 programmable counters)
#if defined(USE_CPU_CYCLES)
            fixed_event.configure(PERF_COUNT_HW_CPU_CYCLES);
            event_list.push_back(fixed_event);
#endif
            fixed_event.configure(PERF_COUNT_HW_REF_CPU_CYCLES);
            event_list.push_back(fixed_event);

            // Add the list of events to the CPU group events
            for (auto& group : m_group_events)
            {
                // Add the events to the CPU group
                group.add(event_list);
            }
        }
        return success;
    }

    bool driver_interface_impl::_collect_telemetry_data(hw_telemetry_records_t& data)
    {

        bool success = false;
        struct epoll_event triggered_events[m_num_cpus] = {};

        // TODO: This will need tuning make it a parameter?
        constexpr std::chrono::milliseconds ONE_SECOND_TIMEOUT(1000);
        const int num_events =
            epoll_wait(m_epollfd, triggered_events, 4 - 1, ONE_SECOND_TIMEOUT.count());
        // When events are received (num_events > 0) or timeout (nun_events == 0) occurs get the
        // sample data
        if (num_events >= 0)
        {
#ifdef DEBUG_VERBOSE
            if (num_events == 0)
            {
                std::cout << "Timeout!" << std::endl;
            }
#endif
            // Get the telemetry records for each CPU
            std::vector<hw_telemetry_records_t> cpu_telemetry_records_list(m_num_cpus);
            size_t num_records = 0;
            for (auto i = 0u; i < m_num_cpus; ++i)
            {
#ifdef DEBUG_VERBOSE
                if (triggered_events[i].events == EPOLLIN)
                {
                    std::cout << "event " << i << " EPOLLIN set for CPU "
                              << triggered_events[i].data.u32 << std::endl;
                }
#endif
                num_records +=
                    m_group_events[i].get_telemetry_records(cpu_telemetry_records_list[i]);
            }

            if (num_records != 0)
            {
                // Consolidate the per CPU telemetry records into a single collection
                data = std::make_shared<std::vector<tdtsdk_pmi_record_t>>(num_records);
                size_t record_count = 0;
                for (const auto& telemetry_records : cpu_telemetry_records_list)
                {
                    // Is there data from the CPU?
                    if (telemetry_records != nullptr && !telemetry_records->empty())
                    {
                        for (const auto& cpu_record : *telemetry_records)
                        {
                            // Copy the un-filtered records to the data vector
                            if (!_is_pid_filtered(cpu_record.pid))
                            {
                                data->at(record_count) = cpu_record;
                                ++record_count;
                            }
                        }
                    }
                }
                // If records were filtered then resize to prevent records of all zeros
                if (data->size() != record_count)
                {
                    data->resize(record_count);
                }
#ifdef DEBUG_VERBOSE
                std::cout << "RECORDS pushed to pipeline " << data->size() << std::endl;
#endif
                success = true;
            }
        }
        else if (num_events == internal::IO_ERROR)
        {
#ifdef DEBUG
            std::cout << "ERROR : " << strerror(errno) << std::endl;
#endif
        }

        return success;
    }

    bool driver_interface_impl::_init_polling()
    {
        bool success = false;
        struct epoll_event ev = {};

        m_epollfd = epoll_create1(0);
        if (m_epollfd != internal::IO_ERROR)
        {
            int status = internal::IO_ERROR;
            for (const auto& group : m_group_events)
            {
                // Register each of the CPU event groups with epoll
                ev.events = EPOLLIN;
                ev.data.u32 = group.cpu();
                status = epoll_ctl(m_epollfd, EPOLL_CTL_ADD, group.file_descriptor(), &ev);

                if (status == internal::IO_ERROR)
                {
#ifdef DEBUG
                    std::cerr << "Failure to create epoll : " << strerror(errno) << std::endl;
#endif
                    break;
                }
            }
            success = status != internal::IO_ERROR;
        }
#ifdef DEBUG
        else
        {
            std::cerr << "Failure to create epoll : " << strerror(errno) << std::endl;
        }
#endif
        return success;
    }

}  // namespace bit_shovel_plugins
