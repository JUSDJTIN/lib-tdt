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
