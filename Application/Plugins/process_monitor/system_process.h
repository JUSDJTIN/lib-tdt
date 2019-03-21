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
