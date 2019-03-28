/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   driver_interface_impl.h
**
**    @brief  User space driver communication API implementation.
**
**
********************************************************************************
*/
#ifndef DRIVER_INTERFACE_IMP_H
#define DRIVER_INTERFACE_IMP_H

#include <cstdint>
#include <string>
#include <memory>
#include <unordered_set>
#include <atomic>
#include <thread>
#include <mutex>

#include "driver_interface.h"
#include "pmu_event.h"
#include "event_group.h"

namespace bit_shovel_plugins
{

    /**
     * @brief Class that exposes driver to user space application.
     */
    class driver_interface_impl : public driver_interface
    {
    public:
        driver_interface_impl();
        virtual ~driver_interface_impl();

        // should probably not be copied
        driver_interface_impl(const driver_interface_impl&) = delete;
        driver_interface_impl& operator=(const driver_interface_impl&) = delete;

        /**
         * @brief Initialize data structures used for the driver interface.
         *
         * Initializes the CPU event groups base on number of CPUs.
         *
         * @return true if successful.
         */
        bool init() override;

        /**
         * @brief Starts the data collection of the perf driver.
         *
         * Initialize the perf event attributes from the PMU Configuration, opens the events and
         * enables recording of the event sample data.
         *
         * @param[in] config Configuration settings
         * @return true if successful.
         */
        bool start(const PMU_CONFIGURATION_NODE& config) override;

        virtual bool stop() override;

        /**
         * @brief Retrieve the telemetry data from the CPU group events.
         *
         * @param[out] data reference to the telemetry data
         * @return true if successful.
         */
        virtual bool read_telemetry_data(hw_telemetry_records_t& data) override;

        /**
         * @brief set PID to filter
         *
         * @param[in] buffer Buffer with the list of pids
         * @param[in] buffer_size The size of the buffer.
         * @return true if successful.
         */
        virtual bool set_process_filter(uint32_t* buffer, const uint32_t buffer_size) override;

        /**
         * @brief Get Driver version, for the perf kernel module the kernel version is returned.
         *
         * @param[out] version Driver version returned
         * @return true if successful.
         */
        virtual bool read_driver_version(TD_VERSION_NODE& version) override;

    private:
        /**
         * @brief Configure the perf events for the PMU Configuration
         *
         * @param[in] pmu_config The pmu configuration from the profile
         * @return true if successful.
         */
        bool _configure_events(const PMU_CONFIGURATION_NODE& config);

        /**
         * @brief Start the data collection, open and enabled the perf events and setup for polling.
         * @return true if successful.
         */
        bool _start_data_collection();

        /**
         * @brief Initialize the epoll for waiting for data
         *
         * @return true if the initialization was successful
         */
        bool _init_polling();

        /**
         * @brief Wait for telemetry records to be available and collect the data.
         *
         * Wait for data available in any of the CPU ring buffers to be signaled (epoll).
         * When data is available or timeout collect the data from all CPU ring buffers,
         * convert it from perf sample records to pmi telemetry records.  Then the per
         * CPU pmi telemetry records are combined and pushed to the pipeline.
         *
         * @param[out] data the telemetry records
         * @return true if successful
         */
        bool _collect_telemetry_data(hw_telemetry_records_t& data);

        /**
         * @brief Check the process filter list for the pid
         * @param pid
         * @return true if the pid is in the process filter list
         */
        inline bool _is_pid_filtered(const pid_t pid);

        // Member Variables
        /**
         * The number of CPUs
         */
        const uint32_t m_num_cpus;
        /**
         * Flag to indicate running state
         */
        std::atomic_bool m_is_running;
        /**
         * The CPU group events
         */
        std::vector<event_group> m_group_events;

        /**
         * The file descriptor for the epoll
         */
        int m_epollfd;

        /**
         * The list of process to filter (whitelist)
         */
        std::unordered_set<pid_t> m_process_filter_set;

        /**
         * Mutual exclusion for filter set
         */
        std::mutex m_filter_mutex;

    };  // class driver_interface_impl

    using perf_driver_interface_impl_ptr = std::shared_ptr<driver_interface_impl>;

}  // namespace bit_shovel_plugins

#endif  // DRIVER_INTERFACE_IMP_H
