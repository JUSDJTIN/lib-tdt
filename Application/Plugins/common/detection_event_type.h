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