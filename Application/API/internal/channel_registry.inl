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
**    @file   channel_registry.inl
**
**    @brief  Class to keep track of which plugins have registered source
**            or sink capabilities during the registration phase.
**
**
********************************************************************************
*/

#include <map>
#include <boost/any.hpp>
#include "plugin_type_ids_info.h"

namespace bit_shovel
{

    //
    // PRIVATE IMPLEMENTATION DETAILS
    //

    namespace internal
    {

        // forward declaration
        template<class T>
        class channel_registration;

        /**
         * @brief PIMPL class for handling construction and tear-down of unique channel_registry(s)
         * @see channel_registry
         */
        class channel_registry_impl
        {
        public:
            channel_registry_impl()
                : type_id_map{}
            {}

            // this class should never be copied
            channel_registry_impl(const channel_registry_impl&) = delete;
            channel_registry_impl& operator=(const channel_registry_impl&) = delete;

            // helper to obtain the channel_registration for type T
            template<class T>
            channel_registration<T>& get_or_create_channel_registration()
            {
                uint32_t type_id = bit_shovel_plugins::get_type_id<T>();
                const auto& it = type_id_map.find(type_id);
                if (it == type_id_map.end())
                {
                    type_id_map[type_id] = std::make_shared<channel_registration<T>>();
                }

                // any exception that occurs duing boost::any_cast will be handled at the parent
                // caller level.
                auto ptr =
                    boost::any_cast<std::shared_ptr<channel_registration<T>>>(type_id_map[type_id]);
                return *(ptr);
            }

            // add a source for a plugin on type T
            template<class T>
            bit_shovel::result_type add_source(const plugin_id_t& plugin_id)
            {
                bit_shovel::result_type result;
                try
                {
                    this->get_or_create_channel_registration<T>().add_source(plugin_id);
                }
                catch (boost::bad_any_cast& e)
                {
                    result.add_failure() << "add_source() " << e.what() << std::endl;
                }
                return result;
            }

            // add a sink for a plugin on type T
            template<class T>
            bit_shovel::result_type add_sink(const plugin_id_t& plugin_id)
            {
                bit_shovel::result_type result;
                try
                {
                    this->get_or_create_channel_registration<T>().add_sink(plugin_id);
                }
                catch (boost::bad_any_cast& e)
                {
                    result.add_failure() << "add_sink() " << e.what() << std::endl;
                }
                return result;
            }

            // map holding type_id to channel_registration<T> objects
            std::map<uint32_t, boost::any> type_id_map;
        };

        /**
         * @brief Class to manage list of what plug-ins are sources/sinks for a given C++ 'type' T
         * in the channel_registry
         */
        template<class T>
        class channel_registration
        {
        public:
            channel_registration()
                : m_sinks()
                , m_sources()
            {}

            // channel_registration class is NOT copy safe due to 'this' being used in global
            // template variable above
            channel_registration(const channel_registration&) = delete;
            channel_registration& operator=(const channel_registration&) = delete;

            // add a sink for a plugin on this channel of type T
            void add_sink(const plugin_id_t& plugin_id)
            {
                m_sinks.push_back(plugin_id);
            }

            // add a source for a plugin on this channel of type T
            void add_source(const plugin_id_t& plugin_id)
            {
                m_sources.push_back(plugin_id);
            }

            // obtain a reference to the stored sinks on this channel of type T
            plugin_id_list_t& sinks()
            {
                return m_sinks;
            }

            // obtain a reference to the stored sources on this channel of type T
            plugin_id_list_t& sources()
            {
                return m_sources;
            }

        private:
            // list of plugins with at least one sync for type T
            plugin_id_list_t m_sinks;

            // list of plugins with at least one source for type T
            plugin_id_list_t m_sources;
        };

        // extra APIs for use by pipeline itself
        class channel_registry_ex : public channel_registry
        {
        public:
            //
            // IMPLENTATION APIs
            //

            // factory method for use by pipeline itself
            plugin_type_registry create_plugin_type_registry(const plugin_id_t& plugin_id)
            {
                return plugin_type_registry(m_private, plugin_id);
            }
        };

        // internal const global to help us return an empty plugin list by reference
        // if no plugins are registered for a type
        const plugin_id_list_t g_empty_plugin_id_list;
    }  // namespace internal

    // see comments with definition in plugin_type_registry class
    template<class T>
    void plugin_type_registry::register_as_sink()
    {
        if (m_result)
        {
            m_result = m_parent_impl->add_sink<T>(m_plugin_id);
        }
    }

    // see comments with definition in plugin_type_registry class
    template<class T>
    void plugin_type_registry::register_as_source()
    {
        if (m_result)
        {
            m_result = m_parent_impl->add_source<T>(m_plugin_id);
        }
    }

    // see comments with definition in channel_registry class
    inline bit_shovel::result_type plugin_type_registry::get_result()
    {
        return m_result;
    }

    // see comments with definition in channel_registry class
    template<class T>
    bool channel_registry::has_channel() const
    {
        return ((m_private->type_id_map.count(bit_shovel_plugins::get_type_id<T>())) > 0);
    }

    // see comments with definition in channel_registry class
    template<class T>
    const plugin_id_list_t& channel_registry::get_sinks() const
    {
        if (this->has_channel<T>())
        {
            // any exception that occurs duing boost::any_cast will be handled at the parent caller
            // level.
            const auto& ptr = boost::any_cast<std::shared_ptr<internal::channel_registration<T>>>(
                m_private->type_id_map[bit_shovel_plugins::get_type_id<T>()]);
            return ptr->sinks();
        }

        return internal::g_empty_plugin_id_list;  // no channel -> return empty set
    }

    // see comments with definition in channel_registry class
    template<class T>
    const plugin_id_list_t& channel_registry::get_sources() const
    {
        if (this->has_channel<T>())
        {
            // any exception that occurs duing boost::any_cast will be handled at the parent caller
            // level.
            auto ptr = boost::any_cast<std::shared_ptr<internal::channel_registration<T>>>(
                m_private->type_id_map[bit_shovel_plugins::get_type_id<T>()]);
            return ptr->sources();
        }

        return internal::g_empty_plugin_id_list;  // no channel -> return empty set
    }

}  // namespace bit_shovel