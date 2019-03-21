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