/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   system_process_impl.h
**
**    @brief   Get the process lists of the Linux system
**
**
********************************************************************************
*/
#ifndef SYSTEM_PROCESS_IMPL_H
#define SYSTEM_PROCESS_IMPL_H

#include <boost/filesystem.hpp>

#include "system_process.h"

namespace bit_shovel_plugins
{

    class system_process_impl : public system_process
    {
    public:
        /**
         * @brief class constructor
         */
        system_process_impl() = default;

        /**
         * @brief class destructor
         */
        virtual ~system_process_impl() = default;

        // Not copyable
        system_process_impl(const system_process_impl&) = delete;
        system_process_impl& operator=(const system_process_impl&) = delete;

        /**
         * @brief Get current running process list
         * @param process_list   List of running process_list
         */
        virtual bool populate_process_list(process_data_list_t& process_list) override;
        virtual bool update_single_process(pid_t pid, process_data_list_t& process_list) override;

        /**
         * @brief Returns the start time of the sidechannel sampling app.
         */
        virtual uint64_t sampling_start_time() override;

        /**
         * @brief get current process id.
         */
        virtual uint32_t current_process_id() override;

    private:
        boost::filesystem::path _get_executable_path_from_proc_fs(pid_t pid);
        bool _is_numeric(const std::string& name);
    };
}  // namespace bit_shovel_plugins
#endif  // SYSTEM_PROCESS_IMPL_H
