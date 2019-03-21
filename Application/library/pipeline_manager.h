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
**    @file   pipeline_manager.h
**
**    @brief  Class to manage the bit shovel pipeline lifecycle
**
**
********************************************************************************
*/

#ifndef BIT_SHOVEL_PIPELINE_MANAGER_H
#define BIT_SHOVEL_PIPELINE_MANAGER_H

#include <set>
#include <vector>
#include <string>

#include "plugin_api.h"
#include "result_type.h"
#include "plugin_type_ids_info.h"

namespace bit_shovel
{
    //
    // TYPES
    //

    // forward declare PIMPL class
    namespace internal
    {
        class pipeline_manager_impl;
    }

    /**
     * @brief Message types so plugins can halt the pipeline
     */
    enum class pipeline_message_type_t
    {
        // cancel everything due to a fatal error
        abort,

        // no error but work is complete
        complete,

        // notify to user
        notify
    };

    /**
     * @brief Message struct so plugins can halt the pipeline
     */
    struct pipeline_message_t
    {
        // what is the message
        pipeline_message_type_t type;

        // who sent the message
        plugin_id_t origin;

        // optional reason why for error tracing
        std::string reason;
    };

    /**
     * @brief Pipeline exit codes
     */
    enum class pipeline_status_t
    {
        idle,
        running,
        stopped,
        canceled,
        aborted,
        complete
    };

    /**
     * @brief Struct to report details about a plugin that stopped execution
     */
    struct pipeline_plugin_exit_details_t
    {
        // who sent the message
        plugin_id_t origin;

        // optional reason why for error tracing
        std::string reason;
    };

    /**
     * @brief The pipeline notification callback signature.
     *
     * @param[in] msg The notification message
     */
    using pipeline_notification_t = std::function<void(const std::string& msg)>;

    //
    // PUBLIC INTERFACE
    //

    /**
     * @brief Class to manage the bit shovel pipeline lifecycle after plugins and configs are loaded
     * from disk
     */
    class pipeline_manager
    {
    public:
        /**
         * @brief Class constructor for pipeline
         * @param plugins A list of bit shovel plugins to use for this pipeline instance along with
         * the configurations they should use
         */
        pipeline_manager(plugin_list_t& plugins);

        /**
         * @brief Initialize & configure plugins, push configs, and start data sources putting the
         * pipeline into a full running state.
         * @return returns a result_type representing the success or failure of the start attempt.
         */
        result_type start();

        /**
         * @brief Stop all data sources and let all current data event chains push through to
         * completion. Blocks until graph is empty.
         */
        void stop();

        /**
         * @brief Stop all data sources and put the graph in a canceled state.
         *        A canceled state means that no new tasks are scheduled even if that means dropping
         * an event that normally would trigger a follow up task. Blocks until graph is empty.
         *
         */
        void cancel();

        /**
         * @brief Blocks calling thread until the graph is fully stopped or canceled. (e.g. Waits
         * for graph to be empty.)
         * @param timeout_ms how long to wait before timing out- default value of zero means wait
         * forever.
         * @return returns the pipeline status after the wait
         */
        pipeline_status_t wait_for_stop(uint32_t timeout_ms = 0);

        /**
         * @brief Returns the current status
         */
        pipeline_status_t status();

        /**
         * @brief Returns information about why a plugin stopped execution
         */
        pipeline_plugin_exit_details_t exit_info();

        /**
         * @brief set callback for notifications.
         *
         * @param[in] callback to send notifications.
         *
         * @return true if successful; false otherwise.
         */
        bool set_notification_callback(pipeline_notification_t callback);

    private:
        //
        // IMPLEMENTATION DETAILS
        //

        // PIMPL class pointer
        std::shared_ptr<internal::pipeline_manager_impl> m_private;
    };
}  // namespace bit_shovel

namespace bit_shovel_plugins
{
    // unique ids for supported types
    enum class pipeline_type_ids
    {
        pipeline_message_id = plugin_type_ids::pipeline_type_id_start,
        pipeline_message_max_id = plugin_type_ids::pipeline_type_id_end,
    };

    // template specialization function to get unique id of a type
    template<>
    inline const uint32_t get_type_id<bit_shovel::pipeline_message_t>()
    {
        return static_cast<uint32_t>(pipeline_type_ids::pipeline_message_id);
    }
}  // namespace bit_shovel_plugins

#endif  // BIT_SHOVEL_PIPELINE_MANAGER_H
