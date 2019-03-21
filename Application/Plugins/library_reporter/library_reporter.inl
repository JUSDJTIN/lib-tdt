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
**    @file   library_reporter.inl
**
**    @brief  Report detection events to the pipeline manager
**
**
********************************************************************************
*/

#include <sstream>
#include <ctime>
#include <boost/dll.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "library_reporter.h"
#include "pmu_publisher_types.h"
#include "pipeline_manager.h"

using namespace tbb::flow;

namespace bit_shovel_plugins
{
    using cve_set_t = boost::optional<
        boost::property_tree::basic_ptree<std::string, std::string, std::less<std::string>>&>;

    namespace internal
    {
        class library_reporter_helper
        {
        public:
            static inline void escape_path_separator(std::string& path)
            {
#ifdef _WIN32  // only in windows escape \ with \\ for JSON output
                boost::replace_all(path,
                    "\\",
                    "\\\\");  // TODO: separate this function in an OS dependent code file
#endif
            }
        };

        class library_reporter_stream_initializer
        {
        public:
            std::stringstream& init(const bit_shovel::plugin_base&,
                const bit_shovel::plugin_config_t&)
            {
                return m_strstream;
            }

        private:
            std::stringstream m_strstream;
        };

        constexpr auto library_reporter_id = "library_reporter";
        constexpr auto library_reporter_description =
            "Notifies heuristic detection events to the pipeline manager in a configured format. "
            "Requires a heuristic plugin to be loaded.";

        template<class stream_initializer_t>
        library_reporter_templ<stream_initializer_t>::library_reporter_templ(
            std::unique_ptr<bit_shovel::plugin_config_t> config,
            stream_initializer_t initializer)
            : bit_shovel::plugin_base(library_reporter_id,
                  library_reporter_description,
                  std::move(config))
            , m_stream_initializer(std::move(initializer))
        {}

        template<class stream_initializer_t>
        void library_reporter_templ<stream_initializer_t>::register_types(
            bit_shovel::plugin_type_registry& registry)
        {
            // include all potential types
            registry.register_as_sink<detection_event_list_t>();
            registry.register_as_sink<detection_event_with_process_name_list_t>();
            registry.register_as_sink<hw_telemetry_records_t>();
        }

        template<class stream_initializer_t>
        void library_reporter_templ<stream_initializer_t>::_output_event(
            std::stringstream& out_stream,
            detection_event_t& event,
            const bool output_process_extra_info)
        {
            if (m_configuration.include_timestamp)
            {  // seconds since epoch: Jan 01 1970. (UTC); time might return -1 and is ok.
                out_stream << "\"timestamp\":" << std::time(nullptr) << ",";
            }
            if (m_configuration.include_plugin_origin)
            {
                out_stream << "\"plugin_origin\":"
                           << "\"" << event.origin << "\",";
            }
            if (m_configuration.include_detection_message)
            {
                out_stream << "\"message\":"
                           << "\"" << event.description << "\",";
            }
            if (m_configuration.include_process_pid)
            {
                out_stream << "\"pid\":" << event.process_id << ",";
            }
            if (m_configuration.include_process_tid)
            {
                out_stream << "\"tid\":" << event.thread_id << ",";
            }
            if (m_configuration.include_severity)
            {
                out_stream << "\"severity\":" << event.confidence << ",";
            }
            if (output_process_extra_info)
            {
                if (m_configuration.include_process_name)
                {
                    out_stream << "\"process_name\":"
                               << "\""
                               << reinterpret_cast<detection_event_with_process_name_t&>(event)
                                      .process_name
                               << "\",";
                }
                if (m_configuration.include_process_path)
                {
                    std::string process_path =
                        reinterpret_cast<detection_event_with_process_name_t&>(event).full_path;
                    library_reporter_helper::escape_path_separator(process_path);
                    out_stream << "\"process_path\":"
                               << "\"" << process_path << "\",";
                }
            }
        }

        template<class stream_initializer_t>
        bool library_reporter_templ<stream_initializer_t>::_throttle_notifications(
            std::stringstream& out_stream,
            const detection_event_t& event)
        {
            bool notify = false;
            if (m_configuration.notify_every_n_detections_per_pid > 1)
            {
                auto entry = m_detections_per_pid.find(event.process_id);
                if (entry == m_detections_per_pid.end())
                {
                    m_detections_per_pid[event.process_id] =
                        detections_per_pid_t{1, event.confidence};
                }
                else
                {
                    ++entry->second.num_detections;
                    entry->second.total_severity += event.confidence;
                    if (entry->second.num_detections >=
                        m_configuration.notify_every_n_detections_per_pid)
                    {
                        out_stream << "{\"detection\":" << m_detection_msg_preamble_cache;
                        out_stream << "\"last_n_detections\":" << entry->second.num_detections
                                   << ",";
                        if (m_configuration.include_severity)
                        {
                            out_stream << "\"avg_severity_of_last_n_detections\":"
                                       << (entry->second.total_severity /
                                              static_cast<float>(entry->second.num_detections))
                                       << ",";
                        }
                        notify = true;
                        m_detections_per_pid.erase(event.process_id);
                    }
                }
            }
            else
            {
                out_stream << "{\"detection\":" << m_detection_msg_preamble_cache;
                notify = true;
            }

            return notify;
        }

        template<class stream_initializer_t>
        void library_reporter_templ<stream_initializer_t>::_load_config(
            bit_shovel::plugin_config_t& config)
        {
            m_profile_name = config.get<std::string>("profile_name");

            m_configuration.include_profile_name =
                config.get<bool>(this->id() + ".include_profile_name", false);
            m_configuration.include_profile_description =
                config.get<bool>(this->id() + ".include_profile_description", false);
            m_configuration.include_profile_author =
                config.get<bool>(this->id() + ".include_profile_author", false);
            m_configuration.include_profile_date =
                config.get<bool>(this->id() + ".include_profile_date", false);
            m_configuration.include_process_name =
                config.get<bool>(this->id() + ".include_process_name", false);
            m_configuration.include_process_path =
                config.get<bool>(this->id() + ".include_process_path", false);
            m_configuration.include_process_pid =
                config.get<bool>(this->id() + ".include_process_pid", false);
            m_configuration.include_process_tid =
                config.get<bool>(this->id() + ".include_process_tid", false);
            m_configuration.include_threat_cve =
                config.get<bool>(this->id() + ".include_threat_cve", false);
            m_configuration.include_threat_class =
                config.get<bool>(this->id() + ".include_threat_class", false);
            m_configuration.include_severity =
                config.get<bool>(this->id() + ".include_severity", false);
            m_configuration.include_timestamp =
                config.get<bool>(this->id() + ".include_timestamp", false);
            m_configuration.include_detection_message =
                config.get<bool>(this->id() + ".include_detection_message", false);
            m_configuration.include_plugin_origin =
                config.get<bool>(this->id() + ".include_plugin_origin", false);

            m_configuration.notify_every_n_detections_per_pid =
                config.get<uint32_t>(this->id() + ".notify_every_n_detections_per_pid", 0);
        }

        template<class stream_initializer_t>
        const std::string library_reporter_templ<stream_initializer_t>::_get_profile_cves_json(
            bit_shovel::plugin_config_t& config)
        {
            std::string profiles_cves_json;
            try
            {
                auto child_node = config.get_child(m_profile_name + ".cve_ids");

                bit_shovel::plugin_config_t temp_config;
                temp_config.add_child("cve_ids", child_node);
                std::stringstream ss;
                boost::property_tree::json_parser::write_json(ss, temp_config, false);
                auto cves = ss.str();
                profiles_cves_json = cves.substr(1, cves.size() - 3) + ",";  // trim extra {}
            }
            catch (std::exception)
            {
                profiles_cves_json = "\"cve_ids\":null,";
            }

            return profiles_cves_json;
        }

        template<class stream_initializer_t>
        void library_reporter_templ<stream_initializer_t>::_init_preamble_cache(
            bit_shovel::plugin_config_t& config)
        {
            // TODO: use boost property tree json builder
            // JSON format
            m_detection_msg_preamble_cache = "{";
            if (m_configuration.include_profile_name)
            {
                m_detection_msg_preamble_cache += ("\"profile_name\":\"" + m_profile_name + "\",");
            }
            if (m_configuration.include_profile_description)
            {
                auto profile_description =
                    config.get<std::string>(m_profile_name + ".description", "");
                m_detection_msg_preamble_cache +=
                    ("\"profile_description\":\"" + profile_description + "\",");
            }
            if (m_configuration.include_profile_author)
            {
                auto profile_author = config.get<std::string>(m_profile_name + ".author", "");
                m_detection_msg_preamble_cache +=
                    ("\"profile_author\":\"" + profile_author + "\",");
            }
            if (m_configuration.include_profile_date)
            {
                auto profile_date = config.get<std::string>(m_profile_name + ".date", "");
                m_detection_msg_preamble_cache += ("\"profile_date\":\"" + profile_date + "\",");
            }
            if (m_configuration.include_threat_cve)
            {
                m_detection_msg_preamble_cache += _get_profile_cves_json(config);
            }
        }

        template<class stream_initializer_t>
        bit_shovel::result_type library_reporter_templ<stream_initializer_t>::init(
            const bit_shovel::channel_registry& registry,
            bit_shovel::data_network& network,
            bit_shovel::data_source_list_t&)
        {

            bit_shovel::result_type result;  // success by default

            auto& config = *this->configuration();

            // TODO: read notification msg format and extra info to be sent with detection from
            // config

            auto& out_stream = m_stream_initializer.init(*this, config);

            // die if we can't log
            if (!out_stream.good())
            {
                result.add_failure() << "Output stream initialization for the " << this->id()
                                     << " plugin failed." << std::endl;
            }

            // in try-catch block as get_sources() and get_sinks can throw exceptions from
            // boost::any_cast
            try
            {
                if (result &&
                    registry.get_sources<detection_event_with_process_name_list_t>().empty() &&
                    registry.get_sources<detection_event_list_t>().empty())
                {
                    result.add_failure()
                        << "Dependency failure: unable to find required source of "
                           "detection_event_list_t or detection_event_with_process_name_list_t."
                        << std::endl;
                }
                if (result)
                {  // BKC Test App support: check for reception of telmetry data and notify.
                    // the following config param needs to be present in profile used by BKC Test
                    // App
                    auto bkc_mode = config.get<bool>(this->id() + ".bkc_mode", false);
                    if (bkc_mode)
                    {
                        bool rfc_norm_plugins_present = false;
                        try
                        {  // in BKC Test app profile, the random forest and normalizer plugin
                           // should not be present.
                            config.get<double>(
                                "random_forest_classifier.attack_probability_threshold");
                            config.get<uint64_t>("normalizer.model.timestamp_delta");
                            rfc_norm_plugins_present = true;
                        }
                        catch (const boost::property_tree::ptree_error&)
                        {}
                        if (!rfc_norm_plugins_present)
                        {
                            // check dependencies - pmu publisher plugin should be available
                            if (registry.get_sources<hw_telemetry_records_t>().empty())
                            {
                                result.add_failure() << "Dependency failure: unable to find "
                                                        "required source of hw_telemetry_records_t."
                                                     << std::endl;
                            }
                            else
                            {
                                auto node =
                                    std::make_shared<function_node<hw_telemetry_records_t, bool>>(
                                        network.graph(),
                                        1,
                                        [this, &network](
                                            const hw_telemetry_records_t& data) -> bool {
                                            if (data != nullptr && !data->empty())
                                            {
                                                network.push<bit_shovel::pipeline_message_t>(
                                                    {bit_shovel::pipeline_message_type_t::notify,
                                                        this->id(),
                                                        "{\"telemetry\": {\"available\": true}}"});
                                            }
                                            return true;
                                        });

                                // register with network
                                // lazy eval means the second term won't run if success == false
                                result = network.add_sink_node<hw_telemetry_records_t>(node);
                            }
                        }
                        else
                        {
                            result.add_failure()
                                << "BKC Test mode: not required plugins are present." << std::endl;
                        }
                    }
                }
                if (result)
                {
                    _load_config(config);
                    _init_preamble_cache(config);
                    // do we have the process_monitor plugin?
                    if (!registry.get_sources<detection_event_with_process_name_list_t>().empty())
                    {
                        // if so, grab the process name for display
                        // add the node to report to pipeline
                        auto node = std::make_shared<
                            function_node<detection_event_with_process_name_list_t>>(
                            network.graph(),
                            1,
                            [this, &out_stream, &network](
                                const detection_event_with_process_name_list_t& events) {
                                for (auto& event : events)
                                {
                                    if (_throttle_notifications(out_stream, event))
                                    {
                                        _output_event(out_stream,
                                            const_cast<detection_event_with_process_name_t&>(event),
                                            true);
                                        out_stream << "\"eof\":null}}" << std::endl;

                                        network.push<bit_shovel::pipeline_message_t>(
                                            {bit_shovel::pipeline_message_type_t::notify,
                                                this->id(),
                                                out_stream.str()});
                                        out_stream.str("");
                                    }
                                }
                            });

                        result =
                            network.add_sink_node<detection_event_with_process_name_list_t>(node);
                    }
                    else
                    {
                        // otherwise, just report raw  detection info
                        // add the node to report to pipeline
                        auto node = std::make_shared<function_node<detection_event_list_t>>(
                            network.graph(),
                            1,
                            [this, &out_stream, &network](const detection_event_list_t& events) {
                                for (auto& event : events)
                                {
                                    if (_throttle_notifications(out_stream, event))
                                    {
                                        _output_event(out_stream,
                                            const_cast<detection_event_t&>(event),
                                            false);
                                        out_stream << "\"eof\":null}}" << std::endl;

                                        network.push<bit_shovel::pipeline_message_t>(
                                            {bit_shovel::pipeline_message_type_t::notify,
                                                this->id(),
                                                out_stream.str()});
                                        out_stream.str("");
                                    }
                                }
                            });

                        result = network.add_sink_node<detection_event_list_t>(node);
                    }
                }
            }
            catch (boost::bad_any_cast& e)
            {
                result.add_failure() << "library_reporter_templ<stream_initializer_t>::init "
                                     << e.what() << std::endl;
            }

            return result;
        }
    }  // namespace internal
}  // namespace bit_shovel_plugins