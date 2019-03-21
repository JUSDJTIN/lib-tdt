/*
 ** *******************************************************************************
 **     Intel Architecture Group
 **     Copyright (C) 2019 Intel Corporation.
 ** ********************************************************************************
 **
 **     INTEL CONFIDENTIAL
 **     This file, software, or program is supplied under the terms of a
 **     license agreement and/or nondisclosure agreement with Intel Corporation
 **     and may not be copied or disclosed except in accordance with the
 **     terms of that agreement.  This file, software, or program contains
 **     copyrighted material and/or trade secret information of Intel
 **     Corporation, and must be treated as such.  Intel reserves all rights
 **     in this material, except as the license agreement or nondisclosure
 **     agreement specifically indicate.
 **
 **     All rights reserved.  No part of this program or publication
 **     may be reproduced, transmitted, transcribed, stored in a
 **     retrieval system, or translated into any language or computer
 **     language, in any form or by any means, electronic, mechanical,
 **     magnetic, optical, chemical, manual, or otherwise, without
 **     the prior written permission of Intel Corporation.
 **
 **     Intel makes no warranty of any kind regarding this code.  This code
 **     is provided on an "As Is" basis and Intel will not provide any support,
 **     assistance, installation, training or other services.  Intel does not
 **     provide any updates, enhancements or extensions.  Intel specifically
 **     disclaims any warranty of merchantability, noninfringement, fitness
 **     for any particular purpose, or any other warranty.
 **
 **     Intel disclaims all liability, including liability for infringement
 **     of any proprietary rights, relating to use of the code.  No license,
 **     express or implied, by estoppel or otherwise, to any intellectual
 **     property rights is granted herein.
 ** /
 */
/********************************************************************************
 **
 **      @file   plugin_loader.cpp
 **
 **      @brief  plugin loading function(s)
 **
 **
 ********************************************************************************/

#include <boost/filesystem/operations.hpp>
#include <boost/function.hpp>
#include <boost/dll.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/algorithm/string.hpp>

#include "tdt_agent.hpp"
#include "pipeline_manager.h"
#include "config_util.h"
#include "plugin_loader.h"
#include "profile_loader.h"
#include "sign_handler_imp.h"

namespace tdt_library
{
    using namespace bit_shovel;
    namespace fs = boost::filesystem;

    using pluginapi_create_t = std::shared_ptr<bit_shovel::plugin_base>(
        std::unique_ptr<bit_shovel::plugin_config_t> config);
    // The following container prevents the libraries from getting unloaded.
    static std::vector<boost::function<pluginapi_create_t>> plugin_handles;

    /**
     * @brief Obtain the filename without extension from a path
     * @param path Path to the file to obtain the name of
     * @return The filename without extension
     */
    static inline std::string get_base_filename(const std::string& path)
    {
        return fs::path(path).stem().string();
    }

    // suffix and prefix to be added to plugin_name to
    // load the library dynamically during load_plugin()
    const auto lib_suffix = boost::dll::shared_library::suffix().string();

#if defined(__linux__)
    constexpr auto lib_prefix = "lib";
#else
    constexpr auto lib_prefix = "";
#endif  // defined(__linux__)

    // TODO: create a class plugin_loder to wrap below functions in Phase 2.

    void unload_all_plugins()
    {
        plugin_handles.clear();  // release the shared pointer reference to the libs to unload them.
    }

    tdt_return_code load_plugin(const bit_shovel::plugin_load_info_t& plugin_load_info,
        std::unique_ptr<bit_shovel::plugin_config_t> config,
        bit_shovel::plugin_list_t& plugins,
        std::ostream& print_target)
    {
        tdt_return_code rtn_code = TDT_ERROR_SUCCESS;
        std::vector<std::string> custom_profiles_list;
        std::vector<std::string> name_config_pair;
        boost::split(name_config_pair, plugin_load_info, boost::is_any_of(":"));

        // did we find a potential custom profile string?
        if (name_config_pair.size() == 2)
        {
            boost::split(custom_profiles_list, name_config_pair[1], boost::is_any_of(","));

            // if you put ':' you need to list at least one profile!
            if (custom_profiles_list.empty() ||
                // no empty string profiles!
                std::any_of(custom_profiles_list.begin(),
                    custom_profiles_list.end(),
                    [](const std::string& x) { return x.empty(); }))
            {
                print_target << "Invalid plugin load string '" << plugin_load_info << "'"
                             << std::endl;
                rtn_code = TDT_ERROR_INVALID_CONFIG;
            }
        }
        else if (name_config_pair.size() > 2)
        {
            print_target << "Invalid plugin load string '" << plugin_load_info << "'" << std::endl;
            rtn_code = TDT_ERROR_INVALID_CONFIG;
        }

        // if we have a custom profile list, load it!
        if (rtn_code == TDT_ERROR_SUCCESS && !custom_profiles_list.empty())
        {
            config = std::unique_ptr<plugin_config_t>{new plugin_config_t()};
            rtn_code = load_profile_list(custom_profiles_list, config, print_target);
        }

        // if we are still good, try to load the plugin
        if (rtn_code == TDT_ERROR_SUCCESS)
        {
            const auto base_name = get_base_filename(name_config_pair[0]);

            // dynamically loading the plugins
            fs::path lib_path(fs::current_path());

            // Append with required prefix and suffix based on OS.
            lib_path /= lib_prefix + base_name + lib_suffix;
            rtn_code = TDT_ERROR_INVALID_PLUGIN;

            if (fs::exists(lib_path))
            {
                // Verify the file signature of the shared library.
                auto& sign_handler = tdt_utility::sign_handler_imp::instance();
                const bool signature_valid = sign_handler.library_is_valid(lib_path);
                if (signature_valid)
                {
                    plugin_handles.push_back(boost::dll::import_alias<pluginapi_create_t>(
                        boost::move(lib_path),                        // path to library
                        "create_plugin",                              // symbol to import
                        boost::dll::load_mode::append_decorations));  // do append extensions and
                                                                      // prefixes

                    plugins.push_back((plugin_handles.back())(std::move(config)));

                    print_target << "Adding plugin '" << base_name << "'" << std::endl;
                    rtn_code = TDT_ERROR_SUCCESS;
                }
                else
                {
                    rtn_code = TDT_ERROR_SIGNVERIFY_FAILED;
                }
            }

            if (TDT_ERROR_SUCCESS != rtn_code)
            {
                print_target << "Cannot find plugin '" << base_name << "', aborting..."
                             << std::endl;
            }
        }

        return rtn_code;
    }
}  // namespace tdt_library