/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
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

            bit_shovel::result_type _handle_bkc_mode(const bit_shovel::channel_registry& registry,
                bit_shovel::data_network& network);

            template<class detection_event_list_t, class detection_t>
            bit_shovel::result_type _configure_node_to_detect_event(
                bit_shovel::data_network& network,
                std::stringstream& out_stream);

            template<class detection_t>
            void _send_notification(const detection_t& event,
                bit_shovel::data_network& network,
                std::stringstream& out_stream);

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