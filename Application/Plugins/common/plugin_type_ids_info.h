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
**    @file   plugin_type_ids_info.h
**
**    @brief  defines unique type_id ranges for types in plugins
**
*********************************************************************************
*/
namespace bit_shovel_plugins
{
#ifndef GET_TYPE_ID_INFO
#    define GET_TYPE_ID_INFO

#    define MAX_TYPES_PER_PLUGIN 20
#    define MAX_TYPES 256

    // enum that defines range to be used by the plugins for the types
    enum plugin_type_ids
    {
        default_type_id = 0,
        test_type_ids_start = 1,
        test_type_ids_end = MAX_TYPES_PER_PLUGIN,
        pipeline_type_id_start = test_type_ids_end + 1,
        pipeline_type_id_end = pipeline_type_id_start + MAX_TYPES_PER_PLUGIN,
        detection_event_type_ids_start = pipeline_type_id_end + 1,
        detection_event_type_ids_end = detection_event_type_ids_start + MAX_TYPES_PER_PLUGIN,
        boise_heuristic_type_ids_start = detection_event_type_ids_end + 1,
        boise_heuristic_type_ids_end = boise_heuristic_type_ids_start + MAX_TYPES_PER_PLUGIN,
        console_reporter_type_ids_start = boise_heuristic_type_ids_end + 1,
        console_reporter_type_ids_end = console_reporter_type_ids_start + MAX_TYPES_PER_PLUGIN,
        csv_telemetry_recorder_type_ids_start = console_reporter_type_ids_end + 1,
        csv_telemetry_recorder_type_ids_end =
            csv_telemetry_recorder_type_ids_start + MAX_TYPES_PER_PLUGIN,
        file_reporter_type_ids_start = csv_telemetry_recorder_type_ids_end + 1,
        file_reporter_type_ids_end = file_reporter_type_ids_start + MAX_TYPES_PER_PLUGIN,
        hotspot_detector_type_ids_start = file_reporter_type_ids_end + 1,
        hotspot_detector_type_ids_end = hotspot_detector_type_ids_start + MAX_TYPES_PER_PLUGIN,
        normalizer_type_ids_start = hotspot_detector_type_ids_end + 1,
        normalizer_type_ids_end = normalizer_type_ids_start + MAX_TYPES_PER_PLUGIN,
        pmu_publisher_type_ids_start = normalizer_type_ids_end + 1,
        pmu_publisher_type_ids_end = pmu_publisher_type_ids_start + MAX_TYPES_PER_PLUGIN,
        process_monitor_type_ids_start = pmu_publisher_type_ids_end + 1,
        process_monitor_type_ids_end = process_monitor_type_ids_start + MAX_TYPES_PER_PLUGIN,
        random_forest_classifier_type_ids_start = process_monitor_type_ids_end + 1,
        random_forest_classifier_type_ids_end =
            random_forest_classifier_type_ids_start + MAX_TYPES_PER_PLUGIN,
        telemetry_data_replayer_type_ids_start = random_forest_classifier_type_ids_end + 1,
        telemetry_data_replayer_type_ids_end =
            telemetry_data_replayer_type_ids_start + MAX_TYPES_PER_PLUGIN,
        core_telemetry_publisher_type_ids_start = telemetry_data_replayer_type_ids_end + 1,
        core_telemetry_publisher_type_ids_end =
            core_telemetry_publisher_type_ids_start + MAX_TYPES_PER_PLUGIN,
        max_allowed_type_id = MAX_TYPES
    };

    // base template function for get_type_id()
    template<class T>
    inline const uint32_t get_type_id()
    {
        static_assert(sizeof(T) == 0, "Use only types which are supported by any of the plugins!");
        return false;
    }

#endif
}  // namespace bit_shovel_plugins