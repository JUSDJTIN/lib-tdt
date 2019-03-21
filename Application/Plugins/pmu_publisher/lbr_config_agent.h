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