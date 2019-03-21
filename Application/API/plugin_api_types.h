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
**    @file   plugin_api_types.h
**
**    @brief  Types for the plugin API
**
**
********************************************************************************
*/

#ifndef BIT_SHOVEL_PLUGIN_API_TYPES_H
#define BIT_SHOVEL_PLUGIN_API_TYPES_H

#include <memory>
#include <vector>
#include <string>

#include <boost/property_tree/ptree.hpp>

namespace bit_shovel
{

    //
    // TYPES
    //

    // forward declaration
    class plugin_base;
    class data_source_base;

    // unique name for each plugin
    using plugin_id_t = std::string;

    // string describing a plugin's id and any unique profiles to use
    using plugin_load_info_t = std::string;

    // plugin key value pair settings
    using plugin_config_t = boost::property_tree::ptree;

    using plugin_id_list_t = std::vector<plugin_id_t>;
    using plugin_load_info_list_t = std::vector<plugin_load_info_t>;
    using plugin_list_t = std::vector<std::shared_ptr<plugin_base>>;
    using plugin_set_t = boost::optional<
        boost::property_tree::basic_ptree<std::string, std::string, std::less<std::string>>&>;
    using data_source_list_t = std::vector<std::shared_ptr<data_source_base>>;

}  // namespace bit_shovel

#endif  // BIT_SHOVEL_PLUGIN_API_TYPES_H
