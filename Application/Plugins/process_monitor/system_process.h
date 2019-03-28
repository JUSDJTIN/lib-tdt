/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   system_process.h
**
**    @brief  Get the process lists of the system
**
**
********************************************************************************
*/
#ifndef SYSTEM_PROCESS_H
#define SYSTEM_PROCESS_H

#include <set>
#include <map>
#include <string>
#include <memory>
#include <stdint.h>

namespace bit_shovel_plugins
{
    using pid_t = uint32_t;
    using pid_list_t = std::set<pid_t>;

    struct process_data_t
    {
        std::string executable_path;
        std::string name;
    };

    using process_data_list_t = std::map<pid_t, process_data_t>;

    class system_process
    {
    public:
        /**
         * @brief Populate a list with current process_list using OS calls
         * @param process_list List to be populated
         */
        virtual bool populate_process_list(process_data_list_t& process_list) = 0;

        /**
         * @brief Update the process data for a single process using OS calls
         * @param process_list Process list in which the PID needs updated information
         */
        virtual bool update_single_process(pid_t pid, process_data_list_t& process_list) = 0;

        /**
         * @brief Returns the start time of the sidechannel sampling app.
         */
        virtual uint64_t sampling_start_time() = 0;

        /**
         * @brief get current process id.
         */
        virtual uint32_t current_process_id() = 0;

        /**
         * @brief class destructor
         */
        virtual ~system_process() = default;
    };

    using system_process_ptr_t = std::shared_ptr<system_process>;
}  // namespace bit_shovel_plugins
#endif  // SYSTEM_PROCESS_H
