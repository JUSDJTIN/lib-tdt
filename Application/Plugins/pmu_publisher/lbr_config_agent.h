/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   lbr_config_agent.h
**
**    @brief  Helper class for creating the LBR configuration for heuristics
**
**
********************************************************************************
*/

#ifndef LBR_CONFIG_AGENT_H
#define LBR_CONFIG_AGENT_H

#include "plugin_api.h"
#include "pmu_publisher_types.h"

namespace bit_shovel_plugins
{

    class lbr_config_agent
    {
    public:
        /** @brief Parses an LBR configuration object into a hw_telemetry_configuration_t object
         * containing the TDT driver configuration.
         *
         * @param config A hw_lbr_config_t object containing the LBR configuration options
         * @param out (OUT): The TDT driver configuration object, which will have LBR options set
         * from the config object
         * @param lbr_config_opts (OUT): A hw_lbr_config_opts_t object which will have the
         * MSR_LBR_SELECT options added which were enabled in the LBR config options
         */
        static bit_shovel::result_type parse(const lbr_config_t& config,
            hw_telemetry_configuration_t& out,
            hw_lbr_config_opts_t& lbr_config_opts);
    };
}  // namespace bit_shovel_plugins

#endif  // LBR_CONFIG_AGENT_H