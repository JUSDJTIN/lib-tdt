/*
********************************************************************************
**    Intel Architecture Group
**    Copyright (C) 2019 Intel Corporation
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
**    @file   library_reporter.h
**
**    @brief  Report detection events to the pipeline manager
**
**
********************************************************************************
*/

#ifndef PLUGIN_LIBRARY_REPORTER_H
#define PLUGIN_LIBRARY_REPORTER_H

#include <unordered_map>
#include "plugin_api.h"
#include "detection_event_type.h"

namespace bit_shovel_plugins
{

    namespace internal
    {
        // properties to configure detection report
        struct library_reporter_config
        {
            bool include_profile_name = true;
            bool include_profile_description = true;
            bool include_profile_author = true;
            bool include_profile_date = false;
            bool include_process_name = true;
            bool include_process_path = true;
            bool include_process_pid = true;
            bool include_process_tid = false;  // thread id
            bool include_threat_cve = true;
            bool include_threat_class = false;
            bool include_severity = true;
            bool include_timestamp = true;
            bool include_detection_message = true;
            bool include_plugin_origin = true;

            uint32_t notify_every_n_detections_per_pid = 0;  // default notify every detection
        };

        struct detections_per_pid_t
        {
            uint32_t num_detections;
            float total_severity;
        };

        // forward declaration
        class library_reporter_stream_initializer;

        // template allows changing stream initializer for unit testing
        template<class stream_initializer_t>
        class library_reporter_templ : public bit_shovel::plugin_base
        {
        public:
            library_reporter_templ(std::unique_ptr<bit_shovel::plugin_config_t> config,
                stream_initializer_t initializer = stream_initializer_t());

            virtual void register_types(bit_shovel::plugin_type_registry& registry) override;

            virtual bit_shovel::result_type init(const bit_shovel::channel_registry& registry,
                bit_shovel::data_network& network,
                bit_shovel::data_source_list_t& data_source_list_out) override;

            // factory method to create a shared_ptr to the class object
            static std::shared_ptr<library_reporter_templ<stream_initializer_t>> create(
                std::unique_ptr<bit_shovel::plugin_config_t> config);

        private:
            void _load_config(bit_shovel::plugin_config_t& config);
            const std::string _get_profile_cves_json(bit_shovel::plugin_config_t& config);
            void _init_preamble_cache(bit_shovel::plugin_config_t& config);
            void _output_event(std::stringstream& out_stream,
                detection_event_t& event,
                const bool output_process_extra_info);
            bool _throttle_notifications(std::stringstream& out_stream,
                const detection_event_t& event);

            stream_initializer_t m_stream_initializer;
            library_reporter_config m_configuration;
            std::string m_profile_name;
            std::string m_detection_msg_preamble_cache;
            std::unordered_map<uint32_t, detections_per_pid_t> m_detections_per_pid;
        };

    }  // namespace internal

    using library_reporter =
        internal::library_reporter_templ<internal::library_reporter_stream_initializer>;

}  // namespace bit_shovel_plugins

// template methods needs to be available to all compilation units
#include "library_reporter.inl"

#endif  // PLUGIN_LIBRARY_REPORTER_H