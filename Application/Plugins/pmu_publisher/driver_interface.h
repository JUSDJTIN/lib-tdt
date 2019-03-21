/*
********************************************************************************
**    Intel Architecture Group
**    Copyright (C) 2010-2018 Intel Corporation
********************************************************************************
**
**    INTEL CONFIDENTIAL
**    This file, software, or program is supplied under the terms of a
**    license agreement and/or nondisclosure agreement with Intel Corporation
**    and may not be copied or disclosed except in accordance with the
**    terms of that agreement.  This file, software, or program contains
**    copyrighted material and/or trade secret information of Intel
**    Corporation, and must be treated as such.  Intel reserves all rights
**    in this material, except as the license agreement or nondisclosure
**    agreement specifically indicate.
**
**    All rights reserved.  No part of this program or publication
**    may be reproduced, transmitted, transcribed, stored in a
**    retrieval system, or translated into any language or computer
**    language, in any form or by any means, electronic, mechanical,
**    magnetic, optical, chemical, manual, or otherwise, without
**    the prior written permission of Intel Corporation.
**
**    Intel makes no warranty of any kind regarding this code.  This code
**    is provided on an "As Is" basis and Intel will not provide any support,
**    assistance, installation, training or other services.  Intel does not
**    provide any updates, enhancements or extensions.  Intel specifically
**    disclaims any warranty of merchantability, noninfringement, fitness
**    for any particular purpose, or any other warranty.
**
**    Intel disclaims all liability, including liability for infringement
**    of any proprietary rights, relating to use of the code.  No license,
**    express or implied, by estoppel or otherwise, to any intellectual
**    property rights is granted herein.
**/
/********************************************************************************
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
