/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   plugin_api.h
**
**    @brief  Base class for plug-ins loaded by bit shovel pipeline
**
********************************************************************************
*/

#ifndef BIT_SHOVEL_PLUGIN_API_H
#define BIT_SHOVEL_PLUGIN_API_H

#include "data_source_base.h"
#include "data_network.h"
#include "result_type.h"
#include "channel_registry.h"
#include "plugin_api_types.h"

namespace bit_shovel
{

    /**
     * @brief Base class for plugins to the pipeline system
     * @remark This serves a generic API for the pipeline to manage plugins
     *
     * -----------------------------------------------------
     * The pipeline has the following lifecycle for plugins
     * -----------------------------------------------------
     * 1) register: all plugins declare all sources and sinks they support (used by plugins to
     * verify dependencies) 2) init: no data is being pushed but all nodes and edges are added to
     * the graph. plugins verify dependency requirements. 3) push_config: plugins which needs to
     * coordinate settings push information to each other in the graph 4) running: all data_sources
     * are started 5) shutdown: all data_sources are stopped and the pipeline waits for all pending
     * task to propagate to completion
     */
    class plugin_base
    {
    public:
        /**
         * @brief Class constructor
         *
         * @param id The unique name to use for this plugin within the pipeline (should be hardcoded
         * when derived)
         * @param description A brief description describing the plugin functionality.
         * @param config The configuration containing various parameters to be set for this plugin.
         */
        plugin_base(const plugin_id_t& id,
            const std::string& description,
            std::unique_ptr<plugin_config_t> config)
            : m_id(id)
            , m_description(description)
            , m_config(std::move(config))
        {}

        /**
         * @brief Obtain a copy of this plugin's uuid
         *
         * @return The unique name to use for this plugin within the pipeline
         */
        plugin_id_t id() const
        {
            return m_id;
        }

        /**
         * @brief Method to retrieve a human readable description of the plugin
         * @return description string
         */
        std::string description() const
        {
            return m_description;
        }

        /**
         * @brief Method to retrieve an accessor to the plugin's configuration
         * @return reference to pointer to the plugin's configuration
         */
        const std::unique_ptr<plugin_config_t>& configuration() const
        {
            return m_config;
        }

        /**
         * @brief Virtual destructor to ensure proper derived class cleanup
         */
        virtual ~plugin_base() = default;

        /**
         * @brief Register supported data interactions by this plugin
         * @param registry Visitor class to enable registration of types to the pipeline
         */
        virtual void register_types(plugin_type_registry& registry) = 0;

        /**
         * @brief Initialize this plugin
         * @remark This method should store any needed persistent config from the input
         * configuration, create any needed nodes and edges on the data_network and return any
         * data_sources the pipeline should activate for this plugin
         *
         * @param network The data_network this plugin should use to add nodes/edges (passed as
         * visitor to init)
         * @param data_source_list_out A list of data_source(s) that should be managed by the
         * pipeline in phases 3/4
         * @return success or error state of attempt
         */
        virtual result_type init(const channel_registry& registry,
            data_network& network,
            data_source_list_t& data_source_list_out) = 0;

        /**
         * @brief Function which will be called during pipeline shutdown after all
         *        data sources have stopped and all pending tasks have finished.
         * @param is_cancel parameter will be true if the pipeline was canceled due to an error
         *        and should not queue new operations.
         */
        virtual void deinit(bit_shovel::data_network& network, const bool is_cancel)
        {
            // disable unreferenced parameter errors in default implemetation
            (void)network;
            (void)is_cancel;
        }

        /**
         * @brief Method called phase 2 of the plugin lifecycle
         * @remark This method should be used to coordinate any configurations needed between
         * plugins (via the graph) before data sources are turned on
         *
         * @param network The data_network this plugin should use to add nodes/edges (passed as
         * visitor to init)
         * @return success or error state of attempt
         */
        virtual result_type push_configs(bit_shovel::data_network& network)
        {
            // disable unreferenced parameter errors in default implemetation
            (void)network;

            return result_type();
        }

    private:
        // The unique name to use for this plugin
        const plugin_id_t m_id;

        // Human readable description of plugin (used for help)
        const std::string m_description;

        // Configuration to use for this plugin
        const std::unique_ptr<plugin_config_t> m_config;
    };

}  // namespace bit_shovel

#endif  // BIT_SHOVEL_PLUGIN_API_H
