/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   library_reporter.cpp
**
**    @brief  Report detection events to the pipeline manager
**
**
********************************************************************************
*/

#include "library_reporter.h"
using namespace bit_shovel_plugins;

namespace bit_shovel_plugins
{
    template<>
    std::shared_ptr<library_reporter> library_reporter::create(
        std::unique_ptr<bit_shovel::plugin_config_t> config)
    {
        return std::make_shared<library_reporter>(std::move(config));
    }

#ifndef DLL_EXPORT
#    define DLL_EXPORT
    BOOST_DLL_ALIAS(library_reporter::create,  // <-- this function is exported with...
        create_plugin)                         // <-- ...this alias name
#endif

}  // namespace bit_shovel_plugins