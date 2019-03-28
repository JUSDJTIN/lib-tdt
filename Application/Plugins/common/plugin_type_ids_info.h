/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
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