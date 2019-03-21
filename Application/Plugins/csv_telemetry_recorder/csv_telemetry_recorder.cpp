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
