/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   data_network.inl
**
**    @brief  Class to manage a data push graph organized into channels for each C++ 'type'
**
**
********************************************************************************
*/

#include <map>
#include <type_traits>
#include <boost/any.hpp>
#include "plugin_type_ids_info.h"

namespace bit_shovel
{

    //
    // PRIVATE IMPLEMENTATION DETAILS
    //

    namespace internal
    {
        /**
         * @brief PIMPL data holding class data for unique data_networks
         * @see data_network
         *
         * @remark This class only holds state for it's owner which is why all the member variables
         * are public. Since it owns state data, it also must execute any cleanup code when the last
         * copy goes away so for that reason cancel() and a destructor have been added to a state
         * holding class.
         */
        class data_network_impl
        {
        public:
            /**
             * @brief Construct a data_network_impl class
             */
            data_network_impl();

            // this class should never be copied
            data_network_impl(const data_network_impl&) = delete;
            data_network_impl& operator=(const data_network_impl&) = delete;

            /**
             *  @brief Class destructor ensures proper graph and task cleanup so we don't crash
             */
            ~data_network_impl();

            /**
             * @brief Block data propagation and wait for all current tasks to flush
             * @see data_network::cancel
             */
            void cancel();

            template<class T>
            inline tbb::flow::broadcast_node<T>& get_or_create_broadcast_node()
            {
                uint32_t type_id = bit_shovel_plugins::get_type_id<T>();
                const auto& it = type_id_map.find(type_id);
                if (it == type_id_map.end())
                {
                    type_id_map[type_id] =
                        std::make_shared<tbb::flow::broadcast_node<T>>(this->graph);
                }

                // any exception that occurs duing boost::any_cast will be handled at the parent
                // caller level.
                auto ptr = boost::any_cast<std::shared_ptr<tbb::flow::broadcast_node<T>>>(
                    type_id_map[type_id]);
                return *(ptr);
            }

            template<class T>
            bool has_broadcast_node() const
            {
                return (static_cast<uint32_t>(
                            type_id_map.count(bit_shovel_plugins::get_type_id<T>())) > 0);
            }

            // keep track of finalized state so we can block new edges being added after plugin init
            // phase
            bool finalized;

            // task context of our graph so we can cancel it
            tbb::task_group_context task_context;

            // graph we are building
            tbb::flow::graph graph;

            // map holding type_id to broadcast_node<T> objects
            std::map<uint32_t, boost::any> type_id_map;

            // keep the nodes alive while this graph exists!
            std::vector<std::shared_ptr<tbb::flow::graph_node>> plugin_nodes;
        };

    }  // namespace internal

    // see comments with definition in data_network class
    template<class T, class node_type>
    result_type data_network::add_source_node(std::shared_ptr<node_type>& node)
    {
        static_assert(std::is_base_of<tbb::flow::graph_node, node_type>::value,
            "Nodes added to data_network must be thread building block flow graph nodes!");
        static_assert(std::is_base_of<tbb::flow::sender<T>, node_type>::value,
            "Attempt to add source node which does not support sending channel type!");

        result_type result;  // success by default

        if (m_private->finalized)
        {
            result.add_failure()
                << "Attempted to add source node when network is already finalized." << std::endl;
        }

        if (result)
        {
            // copy the node to the persistent store
            m_private->plugin_nodes.push_back(node);
            try
            {
                tbb::flow::make_edge<T>(*node, m_private->get_or_create_broadcast_node<T>());
            }
            catch (boost::bad_any_cast& e)
            {
                result.add_failure() << "add_source_node() " << e.what() << std::endl;
            }
        }

        return result;
    }

    // see comments with definition in data_network class
    template<class T, class node_type>
    result_type data_network::add_sink_node(std::shared_ptr<node_type>& node)
    {
        static_assert(std::is_base_of<tbb::flow::graph_node, node_type>::value,
            "Nodes added to data_network must be thread building block flow graph nodes!");
        static_assert(std::is_base_of<tbb::flow::receiver<T>, node_type>::value,
            "Attempt to add sink node which does not support receiving channel type!");

        result_type result;  // success by default

        if (m_private->finalized)
        {
            result.add_failure() << "Attempted to add sink node when network is already finalized."
                                 << std::endl;
        }

        if (result)
        {
            // copy the node to the persistent store
            m_private->plugin_nodes.push_back(node);

            try
            {
                tbb::flow::make_edge<T>(m_private->get_or_create_broadcast_node<T>(), *node);
            }
            catch (boost::bad_any_cast& e)
            {
                result.add_failure() << "add_sink_node() " << e.what() << std::endl;
            }
        }

        return result;
    }

    template<class T, class node_type, class output_port_type>
    result_type data_network::add_multi_source_node(std::shared_ptr<node_type>& multi_node,
        output_port_type& output_port)
    {
        static_assert(std::is_base_of<tbb::flow::graph_node, node_type>::value,
            "Nodes added to data_network must be thread building block flow graph nodes!");
        static_assert(std::is_base_of<tbb::flow::sender<T>, output_port_type>::value,
            "Attempt to add source node which does not support sending channel type!");

        result_type result;  // success by default

        if (m_private->finalized)
        {
            result.add_failure()
                << "Attempted to add source node when network is already finalized." << std::endl;
        }

        if (result)
        {
            // copy the node to the persistent store
            m_private->plugin_nodes.push_back(multi_node);

            try
            {
                tbb::flow::make_edge<T>(output_port, m_private->get_or_create_broadcast_node<T>());
            }
            catch (boost::bad_any_cast& e)
            {
                result.add_failure() << "add_multi_source_node() " << e.what() << std::endl;
            }
        }

        return result;
    }

    // see comments with definition in data_network class
    template<class T>
    result_type data_network::push(const T& value)
    {
        result_type result;  // success by default

        if (m_private->has_broadcast_node<T>())
        {
            try
            {
                auto ptr = boost::any_cast<std::shared_ptr<tbb::flow::broadcast_node<T>>>(
                    m_private->type_id_map[bit_shovel_plugins::get_type_id<T>()]);
                if (!ptr->try_put(value))
                {
                    result.add_failure() << "Broadcast node rejected push attempt." << std::endl;
                }
            }
            catch (boost::bad_any_cast& e)
            {
                result.add_failure() << "data_network::push()  " << e.what() << std::endl;
            }
        }
        else
        {
            result.add_failure() << "Tried to push to a network channel that does not exist."
                                 << std::endl;
        }

        return result;
    }

    template<class T>
    bool data_network::has_broadcast_node() const
    {
        return m_private->has_broadcast_node<T>();
    }

}  // namespace bit_shovel