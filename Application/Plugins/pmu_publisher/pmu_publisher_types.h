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