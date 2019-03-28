/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   data_network.cpp
**
**    @brief  Class to manage the registration of nodes to the tbb flow graph
**
**
********************************************************************************
*/

#include "data_network.h"

namespace bit_shovel
{

    namespace internal
    {

        data_network_impl::data_network_impl()
            : finalized(false)
            , task_context(tbb::task_group_context::isolated,
                  tbb::task_group_context::default_traits)
            , graph(task_context)
            , plugin_nodes()
        {}

        /**
         *  @brief Class destructor ensures proper graph and task cleanup so we don't crash
         */
        data_network_impl::~data_network_impl()
        {
            this->cancel();
        }

        /**
         * @brief Block data propagation and wait for all current tasks to flush
         * @see data_network::cancel
         */
        void data_network_impl::cancel()
        {
            task_context.cancel_group_execution();
            graph.wait_for_all();
        }

    }  // namespace internal

    data_network::data_network()
        : m_private(std::make_shared<decltype(m_private)::element_type>())
    {}

    tbb::flow::graph& data_network::graph()
    {
        return m_private->graph;
    }

    void data_network::finalize()
    {
        m_private->finalized = true;
    }

    bool data_network::is_finalized() const
    {
        return m_private->finalized;
    }

    void data_network::wait_for_completion() const
    {
        m_private->graph.wait_for_all();
    }

    void data_network::cancel()
    {
        m_private->cancel();
    }

}  // namespace bit_shovel
