/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   channel_registry.h
**
**    @brief  Class to keep track of which plugins have registered source
**            or sink capabilities during the registration phase.
**
********************************************************************************
*/

#ifndef BIT_SHOVEL_CHANNEL_REGISTRY_H
#define BIT_SHOVEL_CHANNEL_REGISTRY_H

#include <memory>
#include <vector>

#include "result_type.h"
#include "plugin_api_types.h"

namespace bit_shovel
{
    //
    // TYPES
    //

    /**
     * @brief Id type for internal tracking of unique channel registries
     */
    using channel_registry_id_t = void*;

    // forward declare PIMPL class
    namespace internal
    {
        class channel_registry_impl;
    }

    //
    // PUBLIC INTERFACES ( plugin_type_registry and channel_registry)
    //

    /**
     * @brief Visitor class to help a given plugin register it's possible input and output types
     */
    class plugin_type_registry
    {
    public:
        /**
         * @brief Class constructor for plugin_type_registry
         * @param parent_impl pointer to the internal implementation pimpl class of the parent
         * channel registry
         * @param plugin_id The id of the plugin that will use this class to register
         * @remark This constructor is only used by the pipeline itself and should not be used by
         * plugins themselves
         */
        plugin_type_registry(std::shared_ptr<internal::channel_registry_impl>& parent_impl,
            const plugin_id_t& plugin_id)
            : m_parent_impl(parent_impl)
            , m_plugin_id(plugin_id)
            , m_result()
        {}

        /**
         * @brief Register the plugin passed this visitor as a sink for type T
         */
        template<class T>
        void register_as_sink();

        /**
         * @brief Register the plugin passed this visitor as a source for type T
         */
        template<class T>
        void register_as_source();

        /**
         * @brief  Gets the result_type of the operations performed
         * @return returns the result_type of the operations
         */
        inline bit_shovel::result_type get_result();

    private:
        //
        // IMPLEMENTATION DETAILS
        //

        // Use PIMPL to avoid duplicate shared_ptr counting (address doubles as network id)
        std::shared_ptr<internal::channel_registry_impl>& m_parent_impl;

        // id of parent to register sinks and sources to
        const plugin_id_t m_plugin_id;

        // result_type of the operations performed
        bit_shovel::result_type m_result;
    };

    /**
     * @brief Class to manage registered types for plugins. Types are registered early
     *        so that plugins can check for dependent sources and sinks during init().
     */
    class channel_registry
    {
    public:
        /**
         * @brief Class constructor
         */
        channel_registry();

        /**
         * @brief Method to check if any sources or sinks exist for a given type
         * @return returns true if at least one sink or source is registered for that type
         */
        template<class T>
        bool has_channel() const;

        /**
         * @brief Method to obtain the ids of all plugins registered as a sink for a given type
         * @return returns a list of all plugins ids which are registered as a sink for the given
         * type
         */
        template<class T>
        const plugin_id_list_t& get_sinks() const;

        /**
         * @brief Method to obtain the ids of all plugins registered as a source for a given type
         * @return returns a list of all plugins ids which are registered as a source for the given
         * type
         */
        template<class T>
        const plugin_id_list_t& get_sources() const;

    protected:
        //
        // IMPLEMENTATION DETAILS
        //

        // Use PIMPL to avoid duplicate shared_ptr counting (address doubles as network id)
        std::shared_ptr<internal::channel_registry_impl> m_private;
    };

}  // namespace bit_shovel

// template methods needs to be available to all compilation units
#include "internal/channel_registry.inl"

#endif  // BIT_SHOVEL_CHANNEL_REGISTRY_H
