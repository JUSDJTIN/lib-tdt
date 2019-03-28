/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   driver_interface.h
**
**    @brief  User space driver communication API interface.
**
**
********************************************************************************
*/
#ifndef DRIVER_INTERFACE_H
#define DRIVER_INTERFACE_H

#include <memory>

#include "driver.h"
#include "pmu_publisher_types.h"

using bit_shovel_plugins::hw_telemetry_records_t;

namespace bit_shovel_plugins
{
    /**
     * @brief Interface that expose driver to user space application.
     */
    class driver_interface
    {
    public:
        /**
         * @brief class constructor.
         */
        driver_interface() = default;

        /**
         * @brief class destructor.
         */
        virtual ~driver_interface() = default;

        /**
         * Initialize connections to the Driver APIs
         * @return true if success, false otherwise
         */
        virtual bool init() = 0;

        /**
         * @brief Starts the driver, reserve and check version..
         * @param settings Configuration settings
         * @return true if success, false otherwise.
         */
        virtual bool start(const PMU_CONFIGURATION_NODE& config) = 0;

        /**
         * @brief Stops the driver recording
         * @return true if success, false otherwise.
         */
        virtual bool stop() = 0;

        /**
         * @brief Get the telemetry data from the driver.
         * @return true if success, false otherwise.
         */
        virtual bool read_telemetry_data(hw_telemetry_records_t& data) = 0;

        /**
         * @brief set PID to filter
         * @param buffer Buffer with the list of pids
         * @param buffer_size The size of the buffer.
         * @return true if success, false otherwise.
         */
        virtual bool set_process_filter(uint32_t* buffer, const uint32_t buffer_size) = 0;

        /**
         * @brief Get Driver version
         * @param version Driver version returned
         * @return true if success, false otherwise.
         */
        virtual bool read_driver_version(TD_VERSION_NODE& version) = 0;

    };  // class driver_interface

    using driver_interface_ptr = std::shared_ptr<driver_interface>;

}  // namespace bit_shovel_plugins

#endif  // DRIVER_INTERFACE_H
