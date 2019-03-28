/*
 ********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
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