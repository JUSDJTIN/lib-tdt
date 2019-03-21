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
**    @file   tdt_agent.cpp
**
**    @brief  Implementation of the Threat Detection Technology Library C++ API
**
**
********************************************************************************
*/
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "tdt_agent.hpp"
#include "profile_loader.h"
#include "plugin_loader.h"
#include "config_util.h"
#include "pipeline_manager.h"
#include "plugin_api_types.h"
#include "lib_version.h"
#include "json_utils.h"

using namespace bit_shovel;
using boost::assign::map_list_of;

namespace fs = boost::filesystem;

namespace tdt_library
{
    const std::string empty_string;
    // Following map should be updated if tdt_return_code in tdt_agent.hpp is modified.
    const std::unordered_map<tdt_return_code, const char*> error_strings =
        map_list_of(TDT_ERROR_SUCCESS, "Success")(TDT_ERROR_NULL_PARAM,
            "One or more of required args is null")(TDT_ERROR_INVALID_PARAM,
            "One or more of required args has invalid value")(TDT_ERROR_OUT_OF_MEMORY,
            "Out of memory")(TDT_ERROR_INTERNAL, "An internal error has occurred")(
            TDT_ERROR_INSUFFICIENT_BUFFER_SIZE,
            "Insufficient buffer size")(TDT_ERROR_NOT_IMPLEMENTED,
            "API is not implemented")(TDT_ERROR_STARTUP_FAILURE, "Failed to start TDT agent")(
            TDT_ERROR_INVALID_PLUGIN,
            "Invalid plugin")(TDT_ERROR_INVALID_CONFIG, "Invalid configuration")(
            TDT_ERROR_NO_EXECUTION,
            "Cannot execute")(TDT_ERROR_AGENT_RUNNING, "TDT agent is already running")(
            TDT_ERROR_AGENT_NOT_RUNNING,
            "TDT agent is not running")(TDT_ERROR_AGENT_ABORTED,
            "TDT agent has aborted")(TDT_ERROR_SIGNVERIFY_FAILED, "Failed to verify plugin")(
            TDT_ERROR_NO_PROFILES_AVAILABLE,
            "No profiles are available")(TDT_WARNING_NOT_ALL_PROFILES_LOADED,
            "Not all profiles loaded and parsed successfully")(TDT_ERROR_PROFILES_DIR_NOT_EXISTS,
            "Profiles directory does not exist")(TDT_ERROR_AGENT_UNABLE_TO_STOP,
            "Unable to stop TDT agent");

    // TODO: Replace this with a boost logger
    constexpr std::ostream& print_target = std::cerr;

    /**
     * @brief The implementation class for the Threat Detection Technology Library
     */
    class tdt_agent_impl
    {
    public:
        /**
         * @brief Implementation constructor
         */
        tdt_agent_impl()
            : m_proto_fmt(TDT_PROTO_FORMAT_JSON)
            , m_callback(nullptr)
            , m_context(0)
            , m_active_profiles{}
            , m_configurations{}
            , m_safe_access{}
            , m_configuration_updated(false)
        {}

        /**
         * @brief Implementation constructor
         *
         * @param[in] proto_fmt Protocol format the user wishes to use. JSON is default.
         */
        tdt_agent_impl(const tdt_protocol_format proto_fmt)
            : m_proto_fmt(proto_fmt)
            , m_callback(nullptr)
            , m_context(0)
            , m_active_profiles{}
            , m_configurations{}
            , m_safe_access{}
            , m_configuration_updated(false)
        {}

        /**
         * @brief Implementation destructor
         */
        virtual ~tdt_agent_impl() = default;

        // No Copy constructor or copy assignment
        tdt_agent_impl(const tdt_agent_impl& orig) = delete;
        tdt_agent_impl& operator=(const tdt_agent_impl&) = delete;

        /**
         * @brief discover version, build and supported profiles.
         *
         * @param[out] capabilities on return from this API it will contain a JSON object
         *                          containing version, build, available profiles with properties.
         *
         * @return TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code discover(std::string& capabilities);

        /**
         * @brief get current supported configurations for all profiles or a specific profile.
         *
         * @param[in] opt_profile optional name of profile for which to get the configuration. this
         * can be empty.
         * @param[out] gconfig on return will contain a JSON object describing current
         * configurations for profile(s).
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code get_configuration(const std::string& opt_profile,
            std::string& gconfig);

        /**
         * @brief set configurations for profiles or a specific profile that need to be started when
         * start is called.
         *
         * @param[in] opt_profile optional name of a specific profile for which to set the
         * configuration.
         * @param[in] sconfig a JSON object to modify current configuration for the profile.
         *               if empty then opt_profile can't be empty and current configuration will be
         * applied for profile specified in opt_profile.
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code set_configuration(const std::string& opt_profile,
            const std::string& sconfig);

        /**
         * @brief Start the TDT pipeline
         *
         * @return TDT_ERROR_SUCCESS if the pipeline was successfully started or an error code.
         */
        const tdt_return_code start();

        /**
         * @brief Stop detection
         * @param[in] opt_profile optional name of profile for which to stop detections.
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code
         */
        const tdt_return_code stop(const std::string& opt_profile);

        /**
         * @brief set callback for notifications
         *
         * @param[in] callback to send notifications
         * @param[in] context to send with notifications
         *
         */
        inline void set_notification_callback(notification_t callback, const long long context)
        {
            std::lock_guard<std::mutex> lock(m_safe_access);

            m_callback = callback;
            m_context = context;
        }

    private:
        /**
         * @brief updates properties in current configuration from another configuration.
         *
         * @param[in] curr_config current configuration to update.
         * @param[in] set_config properties from configuration to be updated.
         * @param[in] writable_config_schema the configuration that exports modifiable configuration
         * in curr_config.
         * @return  true on success or false.
         */
        const bool _update_config_properties(const std::shared_ptr<plugin_config_t>& curr_config,
            const plugin_config_t& set_config,
            plugin_config_t& writable_config_schema);

        /**
         * @brief set configurations for multiple profiles that need to be started when start is
         * called.
         *
         * @param[in] sconfig a JSON object containing profiles with configurations to modify.
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code _set_multiple_profiles_config(const std::string& sconfig);

        /**
         * @brief set configurations for specified profile that need to be started when start is
         * called.
         *
         * @param[in] profile name of profile for which to set the configuration.
         * @param[in] sconfig a JSON object to modify current configuration for the profile.
         *               if empty then default configuration will be applied for profile.
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code _set_single_profile_config(const std::string& profile,
            const std::string& sconfig);

        /**
         * @brief set configurations for a profile that need to be started when start is called.
         *
         * @param[in] curr_config current configuration to update.
         * @param[in] json_config a JSON object to modify the current configuration.
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code _set_config_json(const std::shared_ptr<plugin_config_t>& curr_config,
            const std::string& json_config);

        /**
         * @brief creates json object for available profiles.
         *
         * @param profiles_json to fill with the profiles in json format.
         * @return TDT_ERROR_SUCCESS or an error code.
         */
        const tdt_return_code _get_profiles_json(std::string& profiles_json);

        /**
         * @brief creates json object for configurations of available profiles or specified profile.
         *
         * @param opt_profile if not empty then gets configuration for opt_profile otherwise for all
         * available profiles.
         * @param configs_json to fill with the configurations for profiles in json format.
         * @return TDT_ERROR_SUCCESS or an error code.
         */
        const tdt_return_code _get_configs_json(const std::string& opt_profile,
            std::string& configs_json);

        /**
         * @brief checks if pipeline is active in general or for specified profile.
         *
         * @param[in] opt_profile_name optional profile to check if pipeline is active for that
         * profile. can be empty.
         * @return true if pipeline is active in general or for specified profile; false otherwise.
         */
        const bool _is_pipeline_active(const std::string& opt_profile_name);

        /**
         * @brief Searches for profiles in specified directory and loads configurations.
         *
         * @param[in] dir_path directory to search for.
         * @return TDT_ERROR_SUCCESS or an error code.
         */
        const tdt_return_code _load_configs(const fs::path& dir_path);

        /**
         * @brief Start pipeline execution
         *
         * @param[in] plugins A list of plugs to use for this execution
         * @param[in] profile_config the configuration to use for this execution.  Not used!
         * @param[in] print_target A pointer to the output stream for logging
         * @return TDT_ERROR_SUCCESS or an error code.
         */
        const tdt_return_code _run_pipeline(bit_shovel::plugin_list_t& plugins);

        /**
         * @brief Load the plugins
         *
         * @param[in] plugin_set The set of plugins to be loaded
         * @param[in] profile_config the configuration to use for this execution.
         * @return TDT_ERROR_SUCCESS or an error code.
         */
        const tdt_return_code _load_plugins(const plugin_set_t& plugin_set,
            std::unique_ptr<bit_shovel::plugin_config_t>& profile_config);

        const std::string _EXPORT_FOR_GET_SET_CONFIG =
            "export.read_write";  // phase1 export only modifiable properties
        const std::string _PROFILE_NAME_KEY = "profile_name";

        // plugin list to populate
        bit_shovel::plugin_list_t m_plugins = bit_shovel::plugin_list_t();

        // The TDT Pipeline
        std::unique_ptr<bit_shovel::pipeline_manager> m_pipeline = nullptr;

        // The protocol format.
        tdt_protocol_format m_proto_fmt;

        // notifier for detections and error conditions
        notification_t m_callback;

        // The context to send with the notification
        long long m_context;

        // The current active profile list
        std::unordered_set<std::string> m_active_profiles;

        // Profiles configurations
        std::unordered_map<std::string, std::shared_ptr<plugin_config_t>> m_configurations;

        // make it thread-safe
        std::mutex m_safe_access;

        // configuration update status
        bool m_configuration_updated;
    };

    const bool tdt_agent_impl::_is_pipeline_active(const std::string& opt_profile_name)
    {
        bool active = false;

        if (m_pipeline != nullptr && m_pipeline->status() == bit_shovel::pipeline_status_t::running)
        {
            active = opt_profile_name.empty() ||
                     (m_active_profiles.find(opt_profile_name) != m_active_profiles.end());
        }

        return active;
    }

    // TODO: update wiki with profile date format and read_only, read_write configs.
    const tdt_return_code tdt_agent_impl::_load_configs(const fs::path& dir_path)
    {
        constexpr auto PROFILE_EXTENSION = ".profile";
        auto rtn_code = TDT_ERROR_SUCCESS;

        if (fs::exists(dir_path) && fs::is_directory(dir_path))
        {
            for (boost::filesystem::directory_entry& d :
                boost::filesystem::directory_iterator(dir_path))
            {
                fs::path fpath = d.path();
                if (fs::is_regular_file(d) && fpath.extension() == PROFILE_EXTENSION)
                {
                    try
                    {
                        const std::string profile_name = fpath.stem().string();
                        if (m_configurations.find(fpath.string()) ==
                            m_configurations.end())  // only add new
                        {
                            auto config = std::make_shared<plugin_config_t>();
                            boost::property_tree::read_info(
                                fpath.string(), *config);  // load the profile
                            boost::optional<boost::property_tree::ptree&> profile_node =
                                config->get_child_optional(profile_name);
                            if (profile_node)
                            {  // add only profiles with profile name node. other profiles are
                               // invalid or of old format. create profile name property for plugins
                               // to access profile info.
                                config->put<std::string>(_PROFILE_NAME_KEY, profile_name);
                                m_configurations[fpath.string()] = config;
                            }
                        }
                    }
                    catch (const boost::property_tree::ptree_error&)
                    {
                        // flag error and continue to next profile
                        rtn_code = TDT_WARNING_NOT_ALL_PROFILES_LOADED;
                    }
                }
            }
            if (m_configurations.empty())  // make sure atleast one profile was loaded successfully
            {
                rtn_code = TDT_ERROR_NO_PROFILES_AVAILABLE;
            }
        }
        else
        {
            rtn_code = TDT_ERROR_PROFILES_DIR_NOT_EXISTS;
        }

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::_get_profiles_json(std::string& profiles_json)
    {
        auto rtn_code = TDT_ERROR_SUCCESS;
        profiles_json = ",\"profiles\": [";
        for (const auto& profile_info : m_configurations)
        {
            const auto& config = profile_info.second;
            const auto& profile_name = config->get<std::string>(_PROFILE_NAME_KEY);
            const std::string state = _is_pipeline_active(profile_name) ? "active" : "inactive";
            try
            {
                std::stringstream ss;
                auto& child_node = config->get_child(profile_name);
                child_node.put<std::string>("state", state);
                // safe to use write_json for discover since profile node contains all strings.
                boost::property_tree::json_parser::write_json(ss, child_node);

                profiles_json += "{\"" + profile_name + "\":" + ss.str() + "},";
            }
            catch (const boost::property_tree::ptree_error& err)
            {
                if (print_target)
                {
                    print_target << err.what() << std::endl;
                    print_target << "Unable to load '" << profile_info.first << "'." << std::endl;
                    print_target << "Error trying to read profile '" << profile_name << "'."
                                 << std::endl
                                 << std::endl;
                }
                rtn_code = TDT_WARNING_NOT_ALL_PROFILES_LOADED;  // flag error and continue to next
                                                                 // available profile.
            }
        }
        profiles_json += "{\"eof\":null}]";

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::discover(std::string& capabilities)
    {
        std::lock_guard<std::mutex> lock(m_safe_access);
        const auto& profiles_dir = fs::current_path();  // current directory
        // minimum information to return
        capabilities = "{\"version\": \"" TDT_VERSION_STRING "\", \"build\": {\"date\":\"" __DATE__
                       "\",\"time\":\"" __TIME__ "\"}";

        auto rtn_code = _load_configs(profiles_dir);
        if (rtn_code == TDT_ERROR_SUCCESS || rtn_code == TDT_WARNING_NOT_ALL_PROFILES_LOADED)
        {
            std::string profiles_json;
            rtn_code = _get_profiles_json(profiles_json);
            capabilities += profiles_json;  // add even if error condition.
        }
        else
        {
            capabilities += ",\"profiles\": null";
        }

        capabilities += "}";

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::_get_configs_json(const std::string& opt_profile,
        std::string& configs_json)
    {
        auto rtn_code = TDT_ERROR_SUCCESS;
        configs_json += "\"configurations\": [";
        for (const auto& profile_info : m_configurations)
        {
            const auto& config = profile_info.second;
            const auto& profile_name = config->get<std::string>(_PROFILE_NAME_KEY);
            if (!opt_profile.empty() && profile_name != opt_profile)
            {
                continue;
            }
            try
            {
                std::stringstream ss;
                const auto& child_node = config->get_child(
                    _EXPORT_FOR_GET_SET_CONFIG);  // TODO: confirm if read-only configs are needed
                json_utils::output_json(
                    child_node, ss);  // use safe json output function to print in correct format
                configs_json += ("{\"" + profile_name + "\":{" + ss.str() + "}},");
                if (!opt_profile.empty())
                {
                    break;
                }
            }
            catch (const boost::property_tree::ptree_error& err)
            {
                if (print_target)
                {
                    print_target << err.what() << std::endl;
                    print_target << "Unable to load '" << profile_info.first << "'." << std::endl;
                    print_target << "Error trying to read profile '" << profile_name << "'."
                                 << std::endl
                                 << std::endl;
                }
                if (!opt_profile.empty())
                {
                    rtn_code = TDT_ERROR_INVALID_CONFIG;
                    break;
                }

                rtn_code = TDT_WARNING_NOT_ALL_PROFILES_LOADED;  // flag warning and continue to
                                                                 // next available profile.
            }
        }
        configs_json += "{\"eof\":null}]";

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::get_configuration(const std::string& opt_profile,
        std::string& gconfig)
    {
        std::lock_guard<std::mutex> lock(m_safe_access);
        const auto& profiles_dir = fs::current_path();  // current directory
        gconfig = "{";

        auto rtn_code = _load_configs(profiles_dir);
        if (rtn_code == TDT_ERROR_SUCCESS || rtn_code == TDT_WARNING_NOT_ALL_PROFILES_LOADED)
        {
            rtn_code = _get_configs_json(opt_profile, gconfig);
        }

        gconfig += "}";

        return rtn_code;
    }

    /**
     * @brief Implements tree nodes visitor pattern.
     *
     * @param T tree type.
     * @param F function of type bool(*)(PT const& path, T const&) to execute when predicate is
     * satisfied.
     * @param P predicate function of type bool(*)(T const&) invoked for each node.
     * @param PT path type for node path.
     * @return true on success; false if any instance of F invoked on a node returned false.
     * @note visit_if will stop traversing further nodes if F invoked on a node returned false.
     */
    template<typename T, typename F, typename P, typename PT = std::string>
    bool visit_if(T& tree, F const& func, P const& pred, PT const& path = PT())
    {
        bool success = false;
        if (pred(tree))
        {
            success = func(path, tree);
        }
        else
        {
            for (const auto& child : tree)
            {
                if (path.empty())
                {
                    success = visit_if(child.second, func, pred, child.first);
                }
                else
                {
                    success = visit_if(child.second, func, pred, path + "." + child.first);
                }
                if (!success)
                {
                    break;
                }
            }
        }

        return success;
    }

    const bool tdt_agent_impl::_update_config_properties(
        const std::shared_ptr<plugin_config_t>& curr_config,
        const plugin_config_t& set_config,
        plugin_config_t& writable_config_schema)
    {
        return visit_if(set_config,
            [&curr_config, &writable_config_schema](
                std::string const& path, plugin_config_t const& node) -> bool {
                boost::optional<plugin_config_t&> child =
                    writable_config_schema.get_child_optional(path);
                if (!child)
                {
                    return false;  // flag error if property does not exist
                }  // else property found. now update it's value in current configuration.
                curr_config->put(path, node.get_value(""));
                // update writable_config_schema for get_configuration to reflect set value
                writable_config_schema.put(path, node.get_value(""));
                return true;
            },
            [](plugin_config_t const& node) -> bool {  // predicate
                return node.empty();                   // true if leaf node (property)
            });
    }

    const tdt_return_code tdt_agent_impl::_set_config_json(
        const std::shared_ptr<plugin_config_t>& curr_config,
        const std::string& json_config)
    {
        auto rtn_code = TDT_ERROR_INVALID_CONFIG;
        try
        {
            plugin_config_t set_config;
            std::stringstream ss(json_config);
            boost::property_tree::json_parser::read_json(
                ss, set_config);  // json -> internal config
            const auto& profile_name = curr_config->get<std::string>(_PROFILE_NAME_KEY);
            // determine if configuration is contained in a profile name node.
            const auto& it = set_config.find(profile_name);
            if (it != set_config.not_found())
            {
                set_config = set_config.get_child(
                    profile_name);  // extract configuration from profile name node
            }
            // get exported writable configuration for validating and updating properties.
            auto& writable_config_schema = curr_config->get_child(_EXPORT_FOR_GET_SET_CONFIG);
            // update values of properties in curr_config from temp_config based on
            // writable_config_schema.
            if (_update_config_properties(curr_config, set_config, writable_config_schema))
            {
                rtn_code = TDT_ERROR_SUCCESS;
            }
        }
        catch (const boost::property_tree::ptree_error&)
        {}

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::_set_single_profile_config(const std::string& opt_profile,
        const std::string& sconfig)
    {
        auto rtn_code = TDT_ERROR_NO_PROFILES_AVAILABLE;
        // search for profile in loaded configurations
        for (const auto& profile_info : m_configurations)
        {
            const auto& config = profile_info.second;
            const auto& profile_name = config->get<std::string>(_PROFILE_NAME_KEY);
            if (opt_profile == profile_name)
            {
                if (!sconfig.empty())
                {
                    rtn_code = _set_config_json(config, sconfig);
                }
                else
                {  // use default/current configuration for profile
                    rtn_code = TDT_ERROR_SUCCESS;
                }
                if (rtn_code == TDT_ERROR_SUCCESS)
                {
                    m_active_profiles.clear();              // clear current set profiles
                    m_active_profiles.insert(opt_profile);  // add to list of profiles to run
                    m_configuration_updated = true;
                }
                break;
            }  // else continue searching
        }

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::_set_multiple_profiles_config(const std::string& sconfig)
    {
        auto rtn_code = TDT_ERROR_NO_PROFILES_AVAILABLE;
        std::unordered_set<std::string> temp_profiles;
        try
        {
            plugin_config_t set_config;
            std::stringstream ss(sconfig);
            boost::property_tree::json_parser::read_json(
                ss, set_config);  // json -> internal config
            // update configurations for all profiles in sconfig
            for (auto& profile_info : m_configurations)
            {
                auto& curr_config = profile_info.second;
                try
                {
                    const auto& profile_name = curr_config->get<std::string>(_PROFILE_NAME_KEY);
                    // determine if a configuration for profile_name is present in set_config.
                    const auto& it = set_config.find(profile_name);
                    if (it != set_config.not_found())
                    {  // get exported writable configuration for validating and updating
                       // properties.
                        auto& writable_config_schema =
                            curr_config->get_child(_EXPORT_FOR_GET_SET_CONFIG);
                        const auto& temp_config = set_config.get_child(
                            profile_name);  // extract configuration from profile name node
                        // update values of properties in curr_config from temp_config based on
                        // writable_config_schema.
                        if (!_update_config_properties(
                                curr_config, temp_config, writable_config_schema))
                        {
                            rtn_code = TDT_ERROR_INVALID_CONFIG;
                            break;
                        }
                        temp_profiles.insert(profile_name);
                        rtn_code = TDT_ERROR_SUCCESS;
                    }  // continue to next configuration
                }
                catch (const boost::property_tree::ptree_error&)
                {              // old or invalid configuration format
                    continue;  // to next configuration
                }
            }
        }
        catch (const boost::property_tree::ptree_error&)
        {
            rtn_code = TDT_ERROR_INVALID_CONFIG;
        }

        if (rtn_code == TDT_ERROR_SUCCESS)
        {
            m_active_profiles.clear();  // set new profiles to run.
            m_active_profiles = temp_profiles;
            m_configuration_updated = true;
        }

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::set_configuration(const std::string& opt_profile,
        const std::string& sconfig)
    {
        std::lock_guard<std::mutex> lock(m_safe_access);
        if (m_pipeline != nullptr && m_pipeline->status() == bit_shovel::pipeline_status_t::running)
        {
            print_target << "TDT Agent is already running!\n" << std::endl;
            return TDT_ERROR_AGENT_RUNNING;
        }
        const auto& profiles_dir = fs::current_path();  // current directory
        // load configurations just in case discover or get_configuration was not called prior.
        auto rtn_code = _load_configs(profiles_dir);
        if (rtn_code == TDT_ERROR_SUCCESS || rtn_code == TDT_WARNING_NOT_ALL_PROFILES_LOADED)
        {
            if (!opt_profile.empty())
            {
                rtn_code = _set_single_profile_config(opt_profile, sconfig);
            }
            else if (!sconfig.empty())
            {
                rtn_code = _set_multiple_profiles_config(sconfig);
            }
            else
            {  // should not have happened!
                rtn_code = TDT_ERROR_INTERNAL;
            }
        }

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::_load_plugins(const plugin_set_t& plugin_set,
        std::unique_ptr<bit_shovel::plugin_config_t>& profile_config)
    {
        tdt_return_code rtn_code = TDT_ERROR_INTERNAL;

        unload_all_plugins();  // TODO: unload selectively in Phase 2.

        if (plugin_set.get().size() > 0)
        {
            for (const auto& plugin_load_str : plugin_set.get())
            {
                rtn_code = load_plugin(plugin_load_str.second.data(),
                    std::unique_ptr<bit_shovel::plugin_config_t>{
                        new bit_shovel::plugin_config_t(*profile_config)},
                    m_plugins,
                    print_target);
                if (rtn_code != TDT_ERROR_SUCCESS)
                {
                    break;
                }
            }
        }
        else
        {
            print_target
                << "Default plugin set is missing in configuration profile! Aborting run..."
                << std::endl;
            rtn_code = TDT_ERROR_INVALID_CONFIG;
        }

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::start()
    {
        std::lock_guard<std::mutex> lock(m_safe_access);

        if (m_pipeline != nullptr && m_pipeline->status() == bit_shovel::pipeline_status_t::running)
        {
            print_target << "TDT Agent is already running!\n" << std::endl;
            return TDT_ERROR_AGENT_RUNNING;
        }

        if (m_active_profiles.empty())
        {
            print_target << "No profiles have been set to run!\n" << std::endl;
            return TDT_ERROR_NO_PROFILES_AVAILABLE;
        }

        tdt_return_code rtn_code = TDT_ERROR_INTERNAL;

        // Load the plugins
        if (m_configuration_updated)
        {
            m_plugins.clear();  // clear the current plugin list
            for (const auto& profile_info : m_configurations)
            {
                auto config = profile_info.second;
                const auto& profile_name = config->get<std::string>(_PROFILE_NAME_KEY);

                if (m_active_profiles.find(profile_name) != m_active_profiles.end())
                {
                    const plugin_set_t plugin_set =
                        config->get_child_optional(profile_name + ".plugins");
                    if (plugin_set)
                    {
                        auto temp_config =
                            std::unique_ptr<plugin_config_t>{new plugin_config_t(*config)};
                        rtn_code = _load_plugins(plugin_set, temp_config);
                    }
                    else
                    {
                        print_target << "No plugins specified in configuration profile!\n"
                                     << std::endl;
                        rtn_code = TDT_ERROR_INVALID_CONFIG;
                    }
                    break;  // try to load only the first matched profile.
                    // TODO: load multiple profiles in Phase 2.
                }
            }
        }
        else
        {
            rtn_code = TDT_ERROR_SUCCESS;
        }

        if (rtn_code == TDT_ERROR_SUCCESS)
        {
            // start the data graph!
            m_configuration_updated = false;
            rtn_code = _run_pipeline(m_plugins);
        }

        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::_run_pipeline(bit_shovel::plugin_list_t& plugins)
    {
        tdt_return_code rtn_code = TDT_ERROR_SUCCESS;

        m_pipeline = std::unique_ptr<bit_shovel::pipeline_manager>{
            new bit_shovel::pipeline_manager(plugins)};

        if (m_callback)
        {
            m_pipeline->set_notification_callback(
                [this](const std::string& msg) -> void  // C++ function object
                { m_callback(m_context, msg); });
        }

        auto result = m_pipeline->start();
        if (!result)
        {
            // print the error message(s)!
            print_target << result.what();
            print_target << "Unable to start data pipeline!" << std::endl << std::endl;

            return TDT_ERROR_STARTUP_FAILURE;
        }

        print_target << "Executing pipeline..." << std::endl;
        return rtn_code;
    }

    const tdt_return_code tdt_agent_impl::stop(const std::string& opt_profile)
    {
        std::lock_guard<std::mutex> lock(m_safe_access);

        if (m_pipeline == nullptr || m_pipeline->status() != bit_shovel::pipeline_status_t::running)
        {
            print_target << "TDT Agent is NOT running!\n" << std::endl;
            return TDT_ERROR_AGENT_NOT_RUNNING;
        }
        if (opt_profile.empty())
        {  // stop all profiles
            m_pipeline->stop();
        }
        else
        {  // make sure profile is in list of current active profiles
            if (m_active_profiles.find(opt_profile) != m_active_profiles.end())
            {
                // TODO: stop only the specific profile. For now just stop all.
                m_pipeline->stop();
            }
            else
            {
                return TDT_ERROR_NO_PROFILES_AVAILABLE;
            }
        }

        constexpr auto WAIT_TIMEOUT_MS = 100;
        const auto status = m_pipeline->wait_for_stop(WAIT_TIMEOUT_MS);
        auto rtn_code = TDT_ERROR_AGENT_UNABLE_TO_STOP;
        if (status == bit_shovel::pipeline_status_t::stopped)
        {
            rtn_code = TDT_ERROR_SUCCESS;
        }
        else
        {
            print_target << "TDT Agent is not in stopped state!\n" << std::endl;
        }
        // TODO: implement stop/reset for each plugin
        m_configuration_updated = true;  // force unload and reload plugins
        m_pipeline = nullptr;            // force create new pipeline

        return rtn_code;
    }

    //--------------------------------------------------------------------------
    // TDT agent implementation
    //
    const tdt_return_code agent::discover(std::string& capabilities)
    {
        return m_private->discover(capabilities);
    }

    const tdt_return_code agent::get_configuration(const std::string& opt_profile,
        std::string& gconfig)
    {
        return m_private->get_configuration(opt_profile, gconfig);
    }

    const tdt_return_code agent::set_configuration(const std::string& opt_profile,
        const std::string& sconfig)
    {
        if (opt_profile.empty() && sconfig.empty())  // validate args before calling implementation.
        {
            return TDT_ERROR_INVALID_PARAM;
        }

        return m_private->set_configuration(opt_profile, sconfig);
    }

    const tdt_return_code agent::start()
    {
        return m_private->start();
    }

    const tdt_return_code agent::stop(const std::string& opt_profile)
    {
        return m_private->stop(opt_profile);
    }

    const tdt_return_code agent::set_notification_callback(notification_t callback,
        const long long context)
    {
        tdt_return_code status = TDT_ERROR_NULL_PARAM;
        if (callback && context != 0)
        {
            m_private->set_notification_callback(callback, context);
            status = TDT_ERROR_SUCCESS;
        }

        return status;
    }

    const char* agent::get_error_string(const tdt_return_code code)
    {
        return (
            code >= TDT_ERROR_SUCCESS && code < TDT_ERROR_MAX ? error_strings.at(code) : nullptr);
    }

    agent::agent()
        : m_private(std::unique_ptr<tdt_agent_impl>{new tdt_agent_impl()})
    {}

    agent::agent(const tdt_protocol_format proto_fmt)
        : m_private(std::unique_ptr<tdt_agent_impl>{new tdt_agent_impl(proto_fmt)})
    {
        if (proto_fmt != TDT_PROTO_FORMAT_JSON)  // only JSON is supported currently.
        {
            throw std::invalid_argument("protocol format not supported");
        }
    }

    agent::~agent()
    {
        m_private->stop("");
    }
}  // namespace tdt_library