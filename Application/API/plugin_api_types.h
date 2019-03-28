/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
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
