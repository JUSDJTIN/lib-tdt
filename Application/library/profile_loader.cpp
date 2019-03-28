/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   profile_loader.cpp
**
**    @brief  profile processing for the Threat Detection Technology
**
**
********************************************************************************
*/

#include <boost/filesystem/operations.hpp>
#include <boost/function.hpp>
#include <boost/dll.hpp>
#include <boost/property_tree/info_parser.hpp>

#include "tdt_agent.hpp"
#include "pipeline_manager.h"
#include "profile_loader.h"
#include "config_util.h"

namespace tdt_library
{
    // namespaces
    //----------------------------------------------------------------------------
    using namespace bit_shovel;

    // constants
    //----------------------------------------------------------------------------
    constexpr auto profile_filename_extension = ".profile";

    tdt_return_code load_profile_list(const std::vector<std::string>& profiles,
        std::unique_ptr<plugin_config_t>& config_to_populate,
        std::ostream& print_target)
    {
        tdt_return_code rtn_code = TDT_ERROR_SUCCESS;

        config_util::flattened_config_t flat_merged_config;

        for (const auto& profile : profiles)
        {
            if (profile.empty())
            {
                if (print_target)
                {
                    print_target << "Cannot load nameless profile of empty string." << std::endl
                                 << std::endl;
                }
                rtn_code = TDT_ERROR_INVALID_CONFIG;
                break;
            }

            const auto filename = profile + profile_filename_extension;
            if (boost::filesystem::exists(filename))
            {
                plugin_config_t temp_config;

                try
                {
                    boost::property_tree::read_info(filename, temp_config);
                }
                catch (boost::property_tree::info_parser_error& err)
                {
                    if (print_target)
                    {
                        print_target << err.what() << std::endl;
                        print_target << "Unable to load '" << filename << "'." << std::endl;
                        print_target << "Error trying to load profile '" << profile << "'."
                                     << std::endl
                                     << std::endl;
                    }
                    rtn_code = TDT_ERROR_INVALID_CONFIG;
                    break;
                }

                // manually merge temp_configuration into the merged flat configuration
                config_util::overwrite_merge_flattened_config(
                    flat_merged_config, config_util::flatten_config(temp_config));
            }
            else
            {
                if (print_target)
                {
                    print_target << "File '" << filename << "' could not be found." << std::endl;
                    print_target << "Error trying to load profile '" << profile << "'." << std::endl
                                 << std::endl;
                }
                rtn_code = TDT_ERROR_INVALID_CONFIG;
                break;
            }
        }

        // copy the final result out
        if (rtn_code == TDT_ERROR_SUCCESS)
        {
            unflatten_config(flat_merged_config, *config_to_populate);
        }

        return rtn_code;
    }
}  // namespace tdt_library