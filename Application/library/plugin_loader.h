/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**      @file   plugin_loader.h
**
**      @brief  Plugin loader interface description
**
**
********************************************************************************/

#ifndef PLUGINS_H
#define PLUGINS_H

namespace tdt_library
{
    /**
     * @brief Load the requested plugin
     *
     * @param[in] plugin_load_info String describing the plugin name optionally with a custom config
     * profile list
     * @param config Profile to use with this plugin if it doesn't have a custom config profile list
     * @param plugins A container to which the newly loaded plugin should be added
     * @param print_target A reference to the output stream for logging
     * @return Status code for error handling
     */
    tdt_return_code load_plugin(const bit_shovel::plugin_load_info_t& plugin_load_info,
        std::unique_ptr<bit_shovel::plugin_config_t> config,
        bit_shovel::plugin_list_t& plugins,
        std::ostream& print_target);

    /**
     * @brief Unloads all currently loaded plugins.
     *
     */
    void unload_all_plugins();
}  // namespace tdt_library

#endif /* PLUGINS_H */
