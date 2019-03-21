/*
********************************************************************************
**    Intel Architecture Group
**    Copyright (C) 2018 Intel Corporation
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
**    @file   process_monitor.h
**
**    @brief  Read process information about the system for the graph
**
**
********************************************************************************
*/

#ifndef PLUGIN_PROCESS_MONITOR_H
#define PLUGIN_PROCESS_MONITOR_H

#include <mutex>
#include <string>

#include "plugin_api.h"
#include "system_process.h"
#include "system_process_impl.h"

namespace bit_shovel_plugins
{
    using process_filter_list_t = std::vector<std::string>;

    class process_monitor : public bit_shovel::plugin_base
    {
    public:
        process_monitor(std::unique_ptr<bit_shovel::plugin_config_t> config);

        virtual void register_types(bit_shovel::plugin_type_registry& registry) override;

        virtual bit_shovel::result_type init(const bit_shovel::channel_registry& registry,
            bit_shovel::data_network& network,
            bit_shovel::data_source_list_t& data_source_list_out) override;

        // factory method to create a shard_ptr to the class object
        static std::shared_ptr<process_monitor> create(
            std::unique_ptr<bit_shovel::plugin_config_t> config);

    private:
        /**
         * Get the Process name for a pid
         *
         * @param pid the process id
         * @return the process name
         */
        std::string get_process_name(const bit_shovel_plugins::pid_t pid);

        /**
         * Get the Process Path
         *
         * @param pid the process id
         * @return the process name
         */
        std::string get_process_path(const bit_shovel_plugins::pid_t pid);

        /**
         * Get a list of PIDs for the processes with executable path matching 'path'
         *
         * @param[in] path  The path
         * @param[in,out] pids list of pids that match the path
         * @return true when successful
         */
        bool get_pids_by_path(const std::string& path, pid_list_t& pids);

        // process tracking helper
        std::shared_ptr<bit_shovel_plugins::system_process_impl> m_sys_process;

        // thread protection
        std::recursive_mutex m_process_sampler_mutex;

        // current process list
        bit_shovel_plugins::process_data_list_t m_processes;

        // prefix strings to filter processes based on path
        process_filter_list_t m_proc_filter_list;

        // this process pid
        bit_shovel_plugins::pid_t m_pid;

        // configuration cache
        bool m_first_run;
        std::string m_unknown_process_name;
        size_t m_sampling_rate_in_ms;
    };

}  // namespace bit_shovel_plugins

#endif  // PLUGIN_PROCESS_MONITOR_H