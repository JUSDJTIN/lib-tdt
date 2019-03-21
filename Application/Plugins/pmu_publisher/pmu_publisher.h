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
 **    @file   pmu_publisher.h
 **
 **    @brief  Collect hardware telemetry from Intel platforms
 **
 **
 ********************************************************************************
 */

#ifndef PLUGIN_PMU_PUBLISHER_H
#define PLUGIN_PMU_PUBLISHER_H

#include "plugin_api.h"
#include "driver_interface.h"
#include "pmu_publisher_types.h"
#include "pmu_config_agent.h"

namespace bit_shovel_plugins
{
    // forward declare private implementation detail
    namespace internal
    {
        class pmu_publisher_data_source_wrapper;
    }

    class pmu_publisher : public bit_shovel::plugin_base
    {
    public:
        pmu_publisher(std::unique_ptr<bit_shovel::plugin_config_t> config);

        virtual void register_types(bit_shovel::plugin_type_registry& registry) override;

        virtual bit_shovel::result_type init(const bit_shovel::channel_registry& registry,
            bit_shovel::data_network& network,
            bit_shovel::data_source_list_t& data_source_list_out) override;

        virtual bit_shovel::result_type push_configs(bit_shovel::data_network& network) override;

        // factory method to create a shard_ptr to the class object
        static std::shared_ptr<pmu_publisher> create(
            std::unique_ptr<bit_shovel::plugin_config_t> config);

    private:
        bit_shovel_plugins::driver_interface_ptr m_driver_interface;
        hw_telemetry_configuration_t m_driver_config;
        hw_pmu_events_slots_t m_pmu_events_slots;
        hw_lbr_config_opts_t m_lbr_config_opts;
        bool m_send_event_slots;
        bool m_send_lbr_opts;
    };

}  // namespace bit_shovel_plugins

#endif  // PLUGIN_PMU_PUBLISHER_H