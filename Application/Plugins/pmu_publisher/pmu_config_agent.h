/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   pmu_config_agent.h
**
**    @brief  Helper class for creating the PMU configuration for heuristics
**
**
********************************************************************************
*/

#ifndef PLUGIN_PMU_CONFIG_AGENT_H
#define PLUGIN_PMU_CONFIG_AGENT_H

#include "plugin_api.h"
#include "pmu_publisher_types.h"

namespace bit_shovel_plugins
{
    using hw_pmu_events_config_t = std::vector<std::string>;

    class pmu_config_agent
    {
    public:
        /** @brief Parses a PMU configuration object into a hw_telemetry_configuration_t object
         * containing the TDT driver configuration.
         *
         * @param config[in] A hw_pmu_events_config_t object containing the PMU configuration
         * options
         * @param out[out] The PMU configuration object, which will have PMU options set from the
         * config object
         * @param pmu_events_slots[out] This will be populated with event strings from the PMU
         * configuration
         */
        static bit_shovel::result_type parse(const hw_pmu_events_config_t& config,
            hw_telemetry_configuration_t& out,
            hw_pmu_events_slots_t& pmu_events_slots);
    };
}  // namespace bit_shovel_plugins

#endif  // PLUGIN_PMU_CONFIG_AGENT_H