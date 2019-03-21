/*
********************************************************************************
**    Intel Architecture Group
**    Copyright (C) 2018-2019 Intel Corporation
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
