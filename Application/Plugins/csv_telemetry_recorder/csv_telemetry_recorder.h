/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   csv_telemetry_recorder.h
**
**    @brief  Record telemetry data to a csv file
**
**
********************************************************************************
*/

#ifndef PLUGIN_CSV_TELEMETRY_RECORDER_H
#define PLUGIN_CSV_TELEMETRY_RECORDER_H

#include "plugin_api.h"

namespace bit_shovel_plugins
{
    namespace internal
    {
        // forward declaration
        class csv_telemetry_recorder_file_initializer;

        // template allows changing stream initializer for unit testing
        template<class stream_initializer_t>
        class csv_telemetry_recorder_templ : public bit_shovel::plugin_base
        {
        public:
            csv_telemetry_recorder_templ(std::unique_ptr<bit_shovel::plugin_config_t> config,
                stream_initializer_t initializer = stream_initializer_t());

            virtual void register_types(bit_shovel::plugin_type_registry& registry) override;

            virtual bit_shovel::result_type init(const bit_shovel::channel_registry& registry,
                bit_shovel::data_network& network,
                bit_shovel::data_source_list_t& data_source_list_out) override;

            // factory method to create a shard_ptr to the class object
            static std::shared_ptr<csv_telemetry_recorder_templ<stream_initializer_t>> create(
                std::unique_ptr<bit_shovel::plugin_config_t> config);

        private:
            stream_initializer_t m_stream_initializer;
        };
    }  // namespace internal

    using csv_telemetry_recorder =
        internal::csv_telemetry_recorder_templ<internal::csv_telemetry_recorder_file_initializer>;

}  // namespace bit_shovel_plugins

// template methods needs to be available to all compilation units
#include "csv_telemetry_recorder.inl"

#endif  // PLUGIN_CSV_TELEMETRY_RECORDER_H