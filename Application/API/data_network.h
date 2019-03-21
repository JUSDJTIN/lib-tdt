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
**    @file   data_network.h
**
**    @brief  Class to manage a data push graph organized into channels for each C++ 'type'
**
**
********************************************************************************
*/

#ifndef BIT_SHOVEL_DATA_NETWORK_H
#define BIT_SHOVEL_DATA_NETWORK_H

#include <memory>

#include <tbb/flow_graph.h>

#include "result_type.h"

namespace bit_shovel
{

    //
    // TYPES
    //

    /**
     * @brief Id type for internal tracking of unique networks
     */
    using network_id_t = void*;

    // forward declare PIMPL class
    namespace internal
    {
        class data_network_impl;
    }

    //
    // PUBLIC INTERFACE
    //

    /**
     * @brief Class to manage a data push graph organized into channels for each C++ 'type'
     * @remark The data_network class is a visitor to each plugin so that nodes can be added during
     * init
     */
    class data_network
    {
    public:
        /**
         * @brief Class constructor
         */
        data_network();

        /**
         * @brief Method to register a TBB node as a source for a given C++ 'type' channel
         * @remark Method will not compile if node_type is not a graph_node with sender<T> interface
         *
         * @param node A shared_ptr to a TBB::graph_node type which supports sending type T
         * @return returns true if operation was successful
         */
        template<class T, class node_type>
        result_type add_source_node(std::shared_ptr<node_type>& node);

        /**
         * @brief Method to register a TBB node as a sink for a given C++ 'type' channel
         * @remark Method will not compile if node_type is not a graph_node with receiver<T>
         * interface
         *
         * @param node A shared_ptr to a TBB::graph_node type which supports receiving type T
         * @return returns true if operation was successful
         */
        template<class T, class node_type>
        result_type add_sink_node(std::shared_ptr<node_type>& node);

        /**
         * @brief Method to register a multiple output TBB node as a source for a given C++ 'type'
         * channel
         *
         * @param node A shared_ptr to a TBB multifunction_node type which supports receiving type T
         * @return returns true if operation was successful
         */
        template<class T, class node_type, class output_port_type>
        result_type add_multi_source_node(std::shared_ptr<node_type>& multi_node,
            output_port_type& output_port);

        /**
         * @brief Method to manually push a value into the network
         *
         * @param value The value to push
         * @return returns true if push was successful
         */
        template<class T>
        result_type push(const T& value);

        /**
         * @brief A reference to the TBB graph so that TBB nodes can be added to it during plugin
         * init
         *
         * @return reference to the TBB graph object held by this data_network
         */
        tbb::flow::graph& graph();

        /**
         * @brief Lock down the network preventing further sources and sinks from being added
         * @remark This is used to prevent init code from accidentally running after the pipeline
         * init phase is complete
         */
        void finalize();

        /**
         * @brief Check if this network is already finalized
         * @return true if already finalized, otherwise false.
         */
        bool is_finalized() const;

        /**
         * @brief Wait for all data to finish propagating through the network
         * @remark Does not prevent new tasks from being scheduled or data from being pushed
         */
        void wait_for_completion() const;

        /**
         * @brief Prevent nodes from pushing new work or new data from being added and then waits
         * for all tasks to flush
         */
        void cancel();

        /**
         * @brief Method to check if a given type is present in the broadcast_node<T> type_map
         * @return returns true if the given type is present
         */
        template<class T>
        bool has_broadcast_node() const;

    private:
        //
        // IMPLEMENTATION DETAILS
        //

        // Use PIMPL to avoid duplicate shared_ptr counting (address doubles as network id)
        std::shared_ptr<internal::data_network_impl> m_private;
    };

}  // namespace bit_shovel

// template methods needs to be available to all compilation units
#include "internal/data_network.inl"

#endif  // BIT_SHOVEL_DATA_NETWORK_H