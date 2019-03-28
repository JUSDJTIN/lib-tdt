/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   profile_loader.h
**
**    @brief  profile loader interface definition
**
**
********************************************************************************
*/

#ifndef PROFILE_LOADER_H
#define PROFILE_LOADER_H

#include <vector>
#include <memory>
#include <sstream>

#include "plugin_api_types.h"

namespace tdt_library
{
    /**
     * @brief Load the configuration profile in order to build a merged configuration
     *
     * @param[in] profiles A list of string names of the profiles to load
     * @param[in,out] config_to_populate Pointer to a config object to populate the profiles into
     * @param[in] print_target A reference to the output stream for logging
     * @return Status code for error handling
     */
    tdt_return_code load_profile_list(const std::vector<std::string>& profiles,
        std::unique_ptr<bit_shovel::plugin_config_t>& config_to_populate,
        std::ostream& print_target);
}  // namespace tdt_library
#endif /* PROFILE_LOADER_H */
