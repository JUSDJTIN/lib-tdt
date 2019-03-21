/*
********************************************************************************
**    Intel Architecture Group
**    Copyright (C) 2018 Intel Corporation
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
**    @file   data_source_own_thread.h
**
**    @brief  A template class for a data source with its own thread
**
**
********************************************************************************
*/

#ifndef BIT_SHOVEL_DATA_SOURCE_OWN_THREAD_H
#define BIT_SHOVEL_DATA_SOURCE_OWN_THREAD_H

#include <memory>
#include <atomic>
#include <functional>
#include <thread>
#include <condition_variable>

#include "data_source_base.h"

namespace bit_shovel
{
    /**
     * @brief Template helper class for data sources with its own thread in the bit shovel pipeline.
     *        Should be able to cover most basic data source cases without modification.
     */
    template<class T>
    class data_source_own_thread : public bit_shovel::data_source_base
    {
    public:
        /**
         * @brief Definition for data collection function used by this data source
         *
         * @remark The function returns a bool. If false is returned, then the function
         *         will never be called again by the pipeline. If more data will come, always
         *         return true. Also, the simple_data_source will handing calls to stop() for you
         *         so there is no need to handle returning false on stop yourself. You can always
         *         return true unless there is a specific reason to shutdown early. The single
         *         reference parameter is an out parameter for the data collected that should be
         *         pushed into the data network. This parameter is ignored if false is returned.
         */
        using body_function_t = std::function<bool(T&)>;

        /**
         * @brief Type for user function(s) to be called during start/stop events
         */
        using start_stop_function_t = std::function<bit_shovel::result_type()>;

        /**
         * @brief Class constructor
         *
         * @param network A reference to the data_network this source should belong to.
         * @param body A type which can be resolved into a reference to a body_function_t for this
         * type T
         * @see simple_data_source<T>::body_function_t
         */
        template<class F, class G = start_stop_function_t, class H = start_stop_function_t>
        data_source_own_thread(bit_shovel::data_network& network,
            const F& body,
            const G& start = nullptr,
            const H& stop = nullptr)
            : m_body_func(body)
            , m_start_func(start)
            , m_stop_func(stop)
            , m_is_stopped(false)
            , m_is_stop_done(false)
            , m_is_stop_done_cv()
            , m_is_stop_done_mutex()
            , m_network(network)
        {}

        /**
         * @brief Method which instructs the data source to begin pushing data into the graph
         *        Called by the pipeline itself.
         */
        virtual bit_shovel::result_type start() override
        {
            bit_shovel::result_type result;  // success by default

            // call user provider start function if available
            if (m_start_func != nullptr)
            {
                result = m_start_func();
            }
            if (result)
            {
                m_thread = std::move(std::thread([&] { _do_collection(); }));
            }
            return result;
        }

        /**
         * @brief Method which instructs the data source to stop pushing data into the graph
         *        Called by the pipeline itself.
         */
        virtual bit_shovel::result_type stop() override
        {
            bit_shovel::result_type result;  // success by default

            // call user provider stop function if available
            if (m_stop_func != nullptr)
            {
                result = m_stop_func();
            }

            // success == false doesn't prevent stop so keep going

            // update atomic so next call to the harvesting loop will quit
            m_is_stopped = true;

            if ((m_body_func != nullptr) && m_thread.joinable())
            {
                m_thread.join();
            }

            // this is more for reporting than error handling
            // returning failure won't prevent stop of the pipeline
            return result;
        }

    private:
        //
        // IMPLEMENTATION DETAILS
        //

        // Method can be private since a bound private method can be called by another class
        // once the owning class creates a C++11 function pointer and gives it out
        void _do_collection()
        {
            bool keep_going = m_body_func != nullptr;
            if (keep_going)
            {
                // check to see if we are already stopped
                keep_going = !m_is_stopped;
                while (keep_going && !m_is_stopped)
                {
                    T data;
                    keep_going = m_body_func(data);
                    if (keep_going)
                    {
                        m_network.push(data);
                    }
                }
            }
            // if we are stopped or the function returned false,
            // update waiting threads to notify that we are stopped now
            if (!keep_going)
            {
                {
                    std::lock_guard<std::mutex> lk(m_is_stop_done_mutex);
                    m_is_stop_done = true;
                }
                m_is_stop_done_cv.notify_all();
            }
        }

        // member state for handling the tbb source_node over start()/stop()
        body_function_t m_body_func;
        start_stop_function_t m_start_func;
        start_stop_function_t m_stop_func;
        std::atomic_bool m_is_stopped;

        // member state for handling the wait for completion during stop()
        bool m_is_stop_done;
        std::condition_variable m_is_stop_done_cv;
        std::mutex m_is_stop_done_mutex;
        std::thread m_thread;
        bit_shovel::data_network m_network;
    };

}  // namespace bit_shovel

#endif  // BIT_SHOVEL_DATA_SOURCE_OWN_THREAD_H
