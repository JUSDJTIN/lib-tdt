/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   pipeline_manager.cpp
**
**    @brief  Class to manage the lifecycle of the whole bit shovel pipeline
**
**
********************************************************************************
*/

#include "pipeline_manager.h"

#include <memory>
#include <condition_variable>
#include <future>
#include <atomic>
#include <chrono>

namespace bit_shovel
{
    // PIMPL class implementation
    namespace internal
    {
        class pipeline_manager_impl
        {
        public:
            // build the internal representation of the pipeline
            pipeline_manager_impl(plugin_list_t& plugins)
                : m_network()
                , m_data_sources()
                , m_plugins(plugins)
                , m_is_stopped(true)
                , m_wait_for_stop_cv()
                , m_wait_for_stop_mutex()
                , m_future()
                , m_callback()
                , m_status(pipeline_status_t::idle)
                , m_exit_info()
            {
                // reserve ahead for plugin dlls to use
                m_data_sources.reserve(plugins.size());
            }

            // if the last reference to the impl goes away, make sure we clean up
            ~pipeline_manager_impl()
            {
                // clean up our state!
                // see definition of _reset(...) below
                this->_reset(/* do_cancel = */ true, /* rebuild_network = */ false);
            }

            result_type start()
            {
                result_type result;  // success by default

                // can't start nothing
                if (m_plugins.empty())
                {
                    result.add_failure()
                        << "Could not start pipeline because no plugins were specified."
                        << std::endl;
                }

                if (result && m_status == pipeline_status_t::running)
                {
                    result.add_failure()
                        << "Pipeline could not be started because it is already running."
                        << std::endl;
                }

                // reset pipeline state
                m_status = pipeline_status_t::idle;
                m_exit_info = {};
                m_future = decltype(m_future)();

                internal::channel_registry_ex registry;

                if (result)
                {
                    // phase 1: register
                    for (auto& plugin : m_plugins)
                    {
                        plugin_type_registry type_registry =
                            registry.create_plugin_type_registry(plugin->id());
                        plugin->register_types(type_registry);

                        // check to ensure registration succeeded
                        result = type_registry.get_result();
                        if (!result)
                        {
                            break;
                        }
                    }
                }

                if (result)
                {
                    // phase 2: init
                    for (auto& plugin : m_plugins)
                    {
                        result = plugin->init(registry, m_network, m_data_sources);
                        result.add_if_failure()
                            << "Plugin " << plugin->id() << " failed to initialize." << std::endl;
                        if (!result)
                        {
                            break;
                        }
                    }
                }

                if (result)
                {
                    // add pipeline message handling node
                    auto pipeline_msg_node = std::make_shared<
                        tbb::flow::function_node<pipeline_message_t>>(
                        m_network.graph(), 1, [this](const pipeline_message_t& msg) {
                            if (!m_future.valid())
                            {
                                // we have to schedule this in a SEPERATE thread so that the graph
                                // can complete
                                m_future = std::async(std::launch::async, [this, msg]() {
                                    if (msg.type == pipeline_message_type_t::complete)
                                    {
                                        m_exit_info = {msg.origin, msg.reason};
                                        m_status = pipeline_status_t::complete;
                                        this->_reset(
                                            /* do_cancel = */ false, /* rebuild_network = */ true);
                                    }
                                    else if (msg.type == pipeline_message_type_t::abort)  // abort
                                    {
                                        m_exit_info = {msg.origin, msg.reason};
                                        m_status = pipeline_status_t::aborted;
                                        this->_reset(
                                            /* do_cancel = */ true, /* rebuild_network = */ true);
                                    }
                                    else if (msg.type == pipeline_message_type_t::notify)  // notify
                                    {
                                        if (m_callback)
                                        {
                                            m_callback(msg.reason);
                                        }  // else can't even notify error condition as there is no
                                           // callback. TODO: log it!
                                    }
                                    else
                                    {
                                        // this shouldn't happen
                                        m_exit_info = {msg.origin,
                                            "Invalid pipeline message received by pipeline from "
                                            "plugin."};
                                        m_status = pipeline_status_t::aborted;
                                        this->_reset(
                                            /* do_cancel = */ true, /* rebuild_network = */ true);
                                    }
                                });
                            }
                            else
                            {  // still running. handle only notification messages in this thread.
                                if (msg.type == pipeline_message_type_t::notify &&
                                    m_callback)  // notify only if callback available
                                {
                                    // we have to schedule this in a SEPERATE thread so that the
                                    // graph can complete gracefully on stop
                                    m_future = std::async(std::launch::async, [this, msg]() {
                                        if (m_callback)  // make sure callback is still valid
                                        {
                                            m_callback(msg.reason);
                                        }
                                    });
                                }
                            }
                        });

                    result = m_network.add_sink_node<pipeline_message_t>(pipeline_msg_node);
                    result.add_if_failure()
                        << "Unable to add pipeline message handling node during pipeline startup."
                        << std::endl;
                }

                if (result)
                {
                    // no more changes to the network
                    m_network.finalize();

                    // phase 3: config push
                    for (auto& plugin : m_plugins)
                    {
                        result = plugin->push_configs(m_network);
                        result.add_if_failure() << "Plugin " << plugin->id()
                                                << " was unable to push its configs." << std::endl;
                        if (!result) break;
                    }
                }

                if (result)
                {
                    // wait for configs to flush!
                    m_network.wait_for_completion();

                    // if we had a pipeline message
                    // let's wait for it to be processed
                    if (m_future.valid()) m_future.wait();

                    // check for bad network status after config_push
                    if (m_status != pipeline_status_t::idle)
                    {
                        if (!m_exit_info.reason.empty())
                        {
                            result.add_failure() << m_exit_info.reason << std::endl;
                        }

                        if (!m_exit_info.origin.empty())
                        {
                            result.add_failure() << "Plugin " << m_exit_info.origin
                                                 << " ended pipeline execution." << std::endl;
                        }

                        result.add_failure()
                            << "Pipeline error occurred during plugin configuration exchange stage."
                            << std::endl;
                    }
                }

                if (result)
                {
                    // phase 4: start data sources
                    for (auto& source : m_data_sources)
                    {
                        result = source->start();
                        // TODO: add some way to trace this to the parent plugin
                        result.add_if_failure() << "A data source failed to start." << std::endl;
                        if (!result) break;
                    }
                }

                if (result)
                {
                    // we are now running
                    m_status = pipeline_status_t::running;

                    // enable waiting for stop
                    std::lock_guard<std::mutex> lock(m_wait_for_stop_mutex);
                    m_is_stopped = false;
                }
                else
                {
                    // reset pipeline flags
                    m_status = pipeline_status_t::idle;
                    m_exit_info = {};

                    // start() failed. Some of the objects have to be re-created to
                    // get back in a 'good' state for this object so do that now.
                    // see definition of _reset(...) below
                    this->_reset(/* do_cancel = */ true, /* rebuild_network = */ true);
                }

                return result;
            }

            void stop()
            {
                // see definition of _reset(...) below
                this->_reset(/* do_cancel = */ false, /* rebuild_network = */ true);
                m_status = pipeline_status_t::stopped;
            }

            void cancel()
            {
                // see definition of _reset(...) below
                this->_reset(/* do_cancel = */ true, /* rebuild_network = */ true);
                m_status = pipeline_status_t::canceled;
            }

            pipeline_status_t wait_for_stop(uint32_t timeout_ms)
            {
                std::unique_lock<std::mutex> lock(m_wait_for_stop_mutex);
                if (timeout_ms == 0)  // zero means wait forever
                {
                    m_wait_for_stop_cv.wait(lock, [this] { return m_is_stopped; });
                }
                else
                {
                    m_wait_for_stop_cv.wait_for(lock,
                        std::chrono::milliseconds(timeout_ms),
                        [this] { return m_is_stopped; });
                }
                return m_status;
            }

            pipeline_status_t status()
            {
                return m_status;
            }

            pipeline_plugin_exit_details_t exit_info()
            {
                return m_exit_info;
            }

            void set_notification_callback(pipeline_notification_t callback)
            {
                m_callback = callback;
            }

        private:
            // data network
            data_network m_network;
            const std::unique_ptr<plugin_config_t> m_merged_config;

            data_source_list_t m_data_sources;
            plugin_list_t m_plugins;

            // member variables for doing wait_for_stop()
            bool m_is_stopped;
            std::condition_variable m_wait_for_stop_cv;
            std::mutex m_wait_for_stop_mutex;

            // this future is used to create a task
            // to shutdown the pipeline.
            // we have to keep the future as a class member
            // because the destructor of std::future waits
            // for completion. If we wait for the graph
            // to shutdown inside a node, we will deadlock
            // as the graph will wait for the node to finish
            // but the node will wait for the graph to end!
            std::future<void> m_future;

            // the callback to invoke for sending detection notifications
            pipeline_notification_t m_callback;

            // state tracker for the pipeline... mainly used for error handling
            std::atomic<pipeline_status_t> m_status;

            pipeline_plugin_exit_details_t m_exit_info;

            /**
             * @brief Internal helper function to put this object back in a good state.
             *
             * @param do_cancel       Determines if this is a cancel or stop operation.
             * @param rebuild_network Determines if we put the object backed in a 'good' state.
             *                        Should only be 'false' when called from the destructor.
             */
            void _reset(bool do_cancel, bool rebuild_network)
            {
                {
                    std::lock_guard<std::mutex> lock(m_wait_for_stop_mutex);

                    if (!m_is_stopped)
                    {
                        // stop all data sources
                        for (auto& source : m_data_sources)
                        {
                            source->stop();
                        }

                        // is this a cancel?
                        if (do_cancel)
                        {
                            m_network.cancel();
                        }
                        else
                        {
                            m_network.wait_for_completion();
                        }

                        // sources may choose to keep a TBB node alive
                        // and TBB node destructions can fail if
                        // their graph is invalid
                        m_data_sources.clear();

                        // don't re-build the network unless we need to
                        if (rebuild_network)
                        {
                            m_network = data_network();
                        }

                        m_is_stopped = true;
                    }
                }

                m_wait_for_stop_cv.notify_all();
            }
        };

    }  // namespace internal

    // main class

    pipeline_manager::pipeline_manager(plugin_list_t& plugins)
        : m_private(std::make_shared<decltype(m_private)::element_type>(plugins))
    {}

    result_type pipeline_manager::start()
    {
        return m_private->start();
    }

    void pipeline_manager::stop()
    {
        m_private->stop();
    }

    void pipeline_manager::cancel()
    {
        m_private->cancel();
    }

    pipeline_status_t pipeline_manager::wait_for_stop(uint32_t timeout_ms)
    {
        return m_private->wait_for_stop(timeout_ms);
    }

    pipeline_status_t pipeline_manager::status()
    {
        return m_private->status();
    }

    pipeline_plugin_exit_details_t pipeline_manager::exit_info()
    {
        return m_private->exit_info();
    }

    bool pipeline_manager::set_notification_callback(pipeline_notification_t callback)
    {
        if (callback)
        {
            m_private->set_notification_callback(callback);
            return true;
        }

        return false;
    }

}  // namespace bit_shovel
