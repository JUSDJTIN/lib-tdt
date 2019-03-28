/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   detection_event_type.h
**
**    @brief  Public generic type for a detection event
**
**
********************************************************************************
*/

#ifndef PLUGIN_DETECTION_EVENT_TYPE_H
#define PLUGIN_DETECTION_EVENT_TYPE_H

#include <stdint.h>
#include <string>
#include <vector>

#include "plugin_api.h"

namespace bit_shovel_plugins
{

    /**
     * @brief generic detection report from a heuristic
     */
    struct detection_event_t
    {
        detection_event_t(uint64_t timestamp,
            bit_shovel::plugin_id_t origin,
            uint32_t process_id,
            uint32_t detection_type,
            std::string description,
            uint32_t thread_id,
            float confidence)
            : timestamp(timestamp)
            , origin(origin)
            , process_id(process_id)
            , detection_type(detection_type)
            , description(description)
            , thread_id(thread_id)
            , confidence(confidence)
        {}

        // timestamp of data which triggered detection
        uint64_t timestamp;

        // plug-in which issued the detection
        bit_shovel::plugin_id_t origin;

        // info on the detected process
        uint32_t process_id;

        // origin-specific event id
        uint32_t detection_type;

        // string to print to console describing what was detected
        std::string description;

        // info on the detected thread
        uint32_t thread_id;

        // confidence of this detection in %
        float confidence;
    };

    /**
     * @brief detection report with process name string
     */
    struct detection_event_with_process_name_t : public detection_event_t
    {
        detection_event_with_process_name_t(uint64_t timestamp,
            bit_shovel::plugin_id_t origin,
            uint32_t process_id,
            uint32_t detection_type,
            std::string description,
            uint32_t thread_id,
            float confidence,
            std::string process_name,
            std::string full_path)
            : detection_event_t(timestamp,
                  origin,
                  process_id,
                  detection_type,
                  description,
                  thread_id,
                  confidence)
            , process_name(process_name)
            , full_path(full_path)
        {}

        detection_event_with_process_name_t(detection_event_t parent,
            std::string process_name,
            std::string full_path

            )
            : detection_event_t(parent)
            , process_name(process_name)
            , full_path(full_path)
        {}

        std::string process_name;
        std::string full_path;
    };

    /**
     * @brief list of generic detection reports
     */
    using detection_event_list_t = std::vector<detection_event_t>;

    /**
     * @brief list of detection reports with process name strings
     */
    using detection_event_with_process_name_list_t =
        std::vector<detection_event_with_process_name_t>;

    // unique ids for supported types
    enum class detection_event_type_ids
    {
        detection_event_list_id = plugin_type_ids::detection_event_type_ids_start,
        detection_event_with_process_name_list_id,
        detection_event_max_id = plugin_type_ids::detection_event_type_ids_end
    };

    // template specialization function to get unique id of a type
    template<>
    inline const uint32_t get_type_id<bit_shovel_plugins::detection_event_list_t>()
    {
        return static_cast<uint32_t>(detection_event_type_ids::detection_event_list_id);
    }

    template<>
    inline const uint32_t
    get_type_id<bit_shovel_plugins::detection_event_with_process_name_list_t>()
    {
        return static_cast<uint32_t>(
            detection_event_type_ids::detection_event_with_process_name_list_id);
    }
}  // namespace bit_shovel_plugins

#endif  // PLUGIN_DETECTION_EVENT_TYPE_H