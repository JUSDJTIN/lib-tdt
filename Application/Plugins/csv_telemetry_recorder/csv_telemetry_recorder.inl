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
**    @file   csv_telemetry_recorder.cpp
**
**    @brief  Record telemetry data to a csv file
**
**
********************************************************************************
*/

#include <memory>
#include <fstream>

#include <boost/dll.hpp>

#include "csv_telemetry_recorder.h"
#include "csv_telemetry_recorder_types.h"

#include "pmu_publisher_types.h"
#include "simple_data_source.h"
#include "pipeline_manager.h"

using namespace tbb::flow;

namespace bit_shovel_plugins
{

    namespace internal
    {

        // TODO: headers from config!
        const std::vector<std::string> REPORT_HEADER = {"TS",
            "PID",
            "TID",
            "Core",
#ifdef ENABLE_FIXED_COUNTERS
            "Instructions",
            "Cycles",
            "Ref Cycles",
#endif  // ENABLE_FIXED_COUNTERS
#ifdef ENABLE_PROG_COUNTERS
            "Event 0",
            "Event 1",
            "Event 2",
            "Event 3",
#endif  // ENABLE_PROG_COUNTERS
            "64bit"};

        class csv_telemetry_recorder_file_initializer
        {
        public:
            std::ostream& init(const bit_shovel::plugin_base& parent,
                const bit_shovel::plugin_config_t& config)
            {
                file_name = config.get(parent.id() + ".file_name", "output.csv");

                // future compatibility if the app restarts the pipeline without re-creating plugins
                if (m_file_stream.is_open())
                {
                    m_file_stream.flush();
                    m_file_stream.close();
                }

                m_file_stream.open(file_name.c_str(), std::ofstream::out);

                return m_file_stream;
            }

            std::string file_name;

        private:
            std::ofstream m_file_stream;
        };

        constexpr auto csv_telemetry_recorder_id = "csv_telemetry_recorder";
        constexpr auto csv_telemetry_recorder_description =
            "Dumps hardware telemetry data to disk in a csv format. Requires telemetry data input "
            "from another plugin.";

        template<class stream_initializer_t>
        csv_telemetry_recorder_templ<stream_initializer_t>::csv_telemetry_recorder_templ(
            std::unique_ptr<bit_shovel::plugin_config_t> config,
            stream_initializer_t initializer)
            : bit_shovel::plugin_base(csv_telemetry_recorder_id,
                  csv_telemetry_recorder_description,
                  std::move(config))
            , m_stream_initializer(std::move(initializer))
        {}

        template<class stream_initializer_t>
        void csv_telemetry_recorder_templ<stream_initializer_t>::register_types(
            bit_shovel::plugin_type_registry& registry)
        {
            // include all potential types
            registry.register_as_sink<hw_telemetry_records_t>();
        }

        template<class stream_initializer_t>
        bit_shovel::result_type csv_telemetry_recorder_templ<stream_initializer_t>::init(
            const bit_shovel::channel_registry& registry,
            bit_shovel::data_network& network,
            bit_shovel::data_source_list_t&)
        {
            bit_shovel::result_type result;  // success by default

            auto& config = *this->configuration();

            auto separator = config.get(this->id() + ".separator", ",");
            auto enable_blank_line_between_data_batches =
                config.get(this->id() + ".enable_blank_line_between_data_batches", false);
            auto enable_record_latency_logging =
                config.get(this->id() + ".enable_record_latency_logging", false);

            auto& out_stream = m_stream_initializer.init(*this, config);

            // die if we can't log
            if (!out_stream.good())
            {
                result.add_failure() << "Output stream initialization for the " << this->id()
                                     << " plugin failed." << std::endl;
            }

            // in try-catch block as get_sources() and get_sinks can throw exceptions from
            // boost::any_cast
            try
            {
                // check dependencies
                if (result && registry.get_sources<hw_telemetry_records_t>().empty())
                {
                    result.add_failure() << "Dependency failure: unable to find required source of "
                                            "hw_telemetry_records_t."
                                         << std::endl;
                }
            }
            catch (boost::bad_any_cast& e)
            {
                result.add_failure()
                    << "csv_telemetry_recorder_templ<stream_initializer_t>::init() " << e.what()
                    << std::endl;
            }

            if (result)
            {
                // write the headers!
                for (auto iterator = REPORT_HEADER.begin(); iterator != REPORT_HEADER.end();
                     ++iterator)
                {
                    if (iterator != REPORT_HEADER.begin())
                    {
                        out_stream << separator;
                    }
                    out_stream << *iterator;
                }

                out_stream << std::endl;

                // add the node record telemetry data to a csv file
                auto node = std::make_shared<function_node<hw_telemetry_records_t>>(network.graph(),
                    1,
                    [this,
                        &out_stream,
                        enable_blank_line_between_data_batches,
                        separator,
                        enable_record_latency_logging,
                        &network](const hw_telemetry_records_t& data) -> void {
                        // this shouldn't happen, but just in case...
                        if (data == nullptr)
                        {
                            network.push<bit_shovel::pipeline_message_t>(
                                {bit_shovel::pipeline_message_type_t::abort,
                                    this->id(),
                                    "Received null telemetry data set."});
                        }
                        else if (out_stream.good())
                        {
                            for (auto& record : *data)
                            {
                                out_stream << record.timeStamp << separator;
                                out_stream << record.pid << separator;
                                out_stream << record.tid << separator;
                                out_stream << record.cpu;
#ifdef ENABLE_FIXED_COUNTERS
                                for (auto k = 0u; k < MAX_APP_FIXED_EVENTS; k++)
                                {
                                    out_stream << separator << record.fixed_data[k];
                                }
#endif  // ENABLE_FIXED_COUNTERS
#ifdef ENABLE_PROG_COUNTERS
                                for (auto k = 0u; k < MAX_APP_PROG_EVENTS; k++)
                                {
                                    out_stream << separator << record.events_data[k];
                                }
#endif  // ENABLE_PROG_COUNTERS

                                out_stream
                                    << separator
                                    << (uint32_t)record.frame.is_64_bit;  // output is_64_bit to csv

                                if (enable_record_latency_logging)
                                {

// even if the units are the same, linux and windows have different epochs! #TODO
#ifdef _WIN32
                                    FILETIME file_time;
                                    GetSystemTimePreciseAsFileTime(&file_time);

                                    ULARGE_INTEGER large_int;

                                    large_int.LowPart = file_time.dwLowDateTime;
                                    large_int.HighPart = file_time.dwHighDateTime;

                                    uint64_t time = large_int.QuadPart;
#else
                                    uint64_t time = std::chrono::nanoseconds(
                                                        std::chrono::high_resolution_clock::now()
                                                            .time_since_epoch())
                                                        .count() /
                                                    100;
#endif
                                    constexpr auto time_units_per_ms = 10000;

                                    out_stream
                                        << separator
                                        << (time > record.timeStamp
                                                   ? (time - record.timeStamp) / time_units_per_ms
                                                   : 0);
                                }

                                out_stream << std::endl;
                            }

                            if (enable_blank_line_between_data_batches)
                            {
                                out_stream << std::endl;
                            }

                            // we don't want to flush all the time and lose
                            // perf from buffering... the stream will be flushed on stop()
                        }
                    });

                result = network.add_sink_node<hw_telemetry_records_t>(node);
            }

            return result;
        }

    }  // namespace internal

}  // namespace bit_shovel_plugins