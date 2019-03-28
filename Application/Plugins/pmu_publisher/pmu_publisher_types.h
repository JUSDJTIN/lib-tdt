/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   hw_telemetry_harvester_types.h
**
**    @brief  Public types for the hw_telemetry_harvester plugin
**
**
********************************************************************************
*/

#ifndef PLUGIN_HW_TELEMETRY_HARVESTER_TYPES_H
#define PLUGIN_HW_TELEMETRY_HARVESTER_TYPES_H

#include <vector>
#include <list>
#include <string>
#include <array>
#include <map>

#include "driver.h"
#include "lbr_config.h"
#include "plugin_type_ids_info.h"

namespace bit_shovel_plugins
{
    constexpr uint32_t MAX_PROG_COUNTER_SLOTS =
        (sizeof(PMU_CONFIGURATION_NODE::eventSel) / sizeof(PMU_CONFIGURATION_NODE::eventSel[0]));

    // Note: The types in here need to be unique in order for the data to be pushed to only the
    // correct nodes.  An issue was seen where some of the types in here were the same, and the code
    // for a couple of the nodes kept being called in what looked like an infinite loop.

    using hw_pmu_events_slots_t = std::array<std::string, MAX_PROG_COUNTER_SLOTS>;

    using hw_lbr_config_opts_t = std::shared_ptr<std::list<std::string>>;

    using hw_telemetry_records_t = std::shared_ptr<std::vector<tdtsdk_pmi_record_t>>;

    using hw_telemetry_process_list_t = std::shared_ptr<std::vector<uint32_t>>;

    using hw_telemetry_configuration_t = PMU_CONFIGURATION_NODE;

    // unique ids for supported types
    enum class hw_telemetry_type_ids
    {
        hw_pmu_events_slots_id = plugin_type_ids::pmu_publisher_type_ids_start,
        hw_lbr_config_opts_id,
        hw_telemetry_records_id,
        hw_telemetry_process_list_id,
        hw_telemetry_configuration_id,
        hw_telemetry_max_id = plugin_type_ids::pmu_publisher_type_ids_end
    };

    // template specialization function to get unique id of a type
    template<>
    inline const uint32_t get_type_id<bit_shovel_plugins::hw_pmu_events_slots_t>()
    {
        return static_cast<uint32_t>(hw_telemetry_type_ids::hw_pmu_events_slots_id);
    }

    template<>
    inline const uint32_t get_type_id<bit_shovel_plugins::hw_lbr_config_opts_t>()
    {
        return static_cast<uint32_t>(hw_telemetry_type_ids::hw_lbr_config_opts_id);
    }

    template<>
    inline const uint32_t get_type_id<bit_shovel_plugins::hw_telemetry_records_t>()
    {
        return static_cast<uint32_t>(hw_telemetry_type_ids::hw_telemetry_records_id);
    }

    template<>
    inline const uint32_t get_type_id<bit_shovel_plugins::hw_telemetry_process_list_t>()
    {
        return static_cast<uint32_t>(hw_telemetry_type_ids::hw_telemetry_process_list_id);
    }

    template<>
    inline const uint32_t get_type_id<bit_shovel_plugins::hw_telemetry_configuration_t>()
    {
        return static_cast<uint32_t>(hw_telemetry_type_ids::hw_telemetry_configuration_id);
    }
}  // namespace bit_shovel_plugins

#endif  // PLUGIN_HW_TELEMETRY_HARVESTER_TYPES_H