/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   csv_telemetry_recorder.cpp
**
**    @brief  Record telemetry data to a csv file
**
**
********************************************************************************
*/

#include "csv_telemetry_recorder.h"

using namespace bit_shovel_plugins;

namespace bit_shovel_plugins
{
    template<>
    std::shared_ptr<csv_telemetry_recorder> csv_telemetry_recorder::create(
        std::unique_ptr<bit_shovel::plugin_config_t> config)
    {
        return std::make_shared<csv_telemetry_recorder>(std::move(config));
    }

#ifndef DLL_EXPORT
#    define DLL_EXPORT
    BOOST_DLL_ALIAS(csv_telemetry_recorder::create,  // <-- this function is exported with...
        create_plugin)                               // <-- ...this alias name
#endif

}  // namespace bit_shovel_plugins
