/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   lbr_config.h
**
**    @brief  A structure for containing values for the LBR configuration
**
**
********************************************************************************
*/

#ifndef PLUGIN_PMU_PUBLISHER_LBR_CONFIG_H
#define PLUGIN_PMU_PUBLISHER_LBR_CONFIG_H

#include <string>

namespace bit_shovel_plugins
{
    struct lbr_config_t
    {
        lbr_config_t()
            : lbr_enable(false)
            , num_lbrs_to_collect(0)
        {}

        bool lbr_enable;
        std::string msr_lbr_select_config;
        unsigned int num_lbrs_to_collect;
    };
}  // namespace bit_shovel_plugins

#endif  // PLUGIN_PMU_PUBLISHER_LBR_CONFIG_H