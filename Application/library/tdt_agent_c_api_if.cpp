/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   tdt_agent_c_api_if.cpp
**
**    @brief  Implementation of the Threat Detection Technology Library C/C++ glue
**
**
********************************************************************************
*/

#include <unordered_map>
#include <memory>
#include "tdt_agent.h"
#include "tdt_agent.hpp"

#ifndef _WIN32
#    include <cstring>

int memcpy_s(void* dest, size_t, const void* src, size_t count)
{
    return std::memcpy(dest, src, count) != nullptr ? 0 : EPERM;
}
#endif

static_assert((uint32_t)::tdt_return_code::TDT_ERROR_MAX == tdt_library::TDT_ERROR_MAX,
    "C API tdt_return_code not up to date!");
static_assert(
    (uint32_t)::tdt_protocol_format::TDT_PROTO_FORMAT_MAX == tdt_library::TDT_PROTO_FORMAT_MAX,
    "C API tdt_protocol_format not up to date!");

/*
 * The following map is used to associate agentc objects and handles.
 * Handles are opaque representation of the agentc objects.
 * The map can store upto UINT64_MAX handles.
 */
static std::unordered_map<uint64_t, std::shared_ptr<agentc>> handle_agentc_obj_map;

/**
 * @brief Returns an agentc object associated with a handle.
 *
 * @param[in] hagent the handle associated with the agentc object to be returend.
 * @return agentc object if found otherwise nullptr.
 */
static std::shared_ptr<agentc> get_agentc(const agent_handle hagent)
{
    auto handle = reinterpret_cast<uint64_t>(hagent);
    auto agentc_it = handle_agentc_obj_map.find(handle);
    if (agentc_it != handle_agentc_obj_map.end())
    {
        return agentc_it->second;
    }

    return nullptr;
}

/**
 * @brief Creates and returns a handle for a new agentc object.
 *
 * @param[in] agentc_obj the agentc object for which a handle needs to be created.
 * @return valid handle on success or 0 on failure.
 */
static const uint64_t get_handle(const std::shared_ptr<agentc>& agentc_obj)
{
    static uint64_t new_handle = 0;
    static constexpr uint32_t MAX_RETRIES_ON_COLLISION = 3;
    uint32_t retries = MAX_RETRIES_ON_COLLISION;

    if (new_handle == UINT64_MAX)  // prevent overflow
    {
        new_handle = 0;
    }
    do
    {
        ++new_handle;  // next available slot
        auto agentc_it = handle_agentc_obj_map.find(new_handle);
        if (agentc_it == handle_agentc_obj_map.end())
        {
            handle_agentc_obj_map[new_handle] = agentc_obj;  // store object in free slot
            return new_handle;
        }
        --retries;
    } while (retries != 0);
    return 0;
}

/**
 * @brief Releases an existing handle and destructs the object assiociated with it.
 *
 * @param[in] hagent the handle to be released.
 * @return true on success; false otherwise.
 */
static const bool remove_handle(const agent_handle hagent)
{
    auto handle = reinterpret_cast<uint64_t>(hagent);
    auto agentc_it = handle_agentc_obj_map.find(handle);
    if (agentc_it == handle_agentc_obj_map.end())
    {
        return false;
    }
    agentc_it->second = nullptr;
    handle_agentc_obj_map.erase(handle);
    return true;
}

extern "C"
{

    /* This is an internal helper class to glue callback mechanism
     *  between C and C++ worlds.
     */
    struct agentc : public tdt_library::agent
    {
        ::notification_t notify;

        agentc(tdt_library::tdt_protocol_format proto_fmt)
            : agent(proto_fmt)
            , notify{}
        {}

        /* This method will be used from C world to create a std::function<> object to the
         * callback in our native API
         */
        tdt_library::tdt_return_code set_notification_callback(::notification_t notification,
            const long long context)
        {
            notify =
                notification;  // save the C notification callback to use in the C++ function object
            return tdt_library::agent::set_notification_callback(  // call parent function
                [this](const long long context,
                    const std::string& msg) -> void {  // C++ function object
                    notify(context, msg.data(), msg.length());
                },
                context);
        }
    };

    const ::tdt_return_code discover(const agent_handle hagent,
        char* capabilities,
        const size_t caps_buffer_len,
        size_t* caps_len)
    {
        tdt_library::tdt_return_code ret_val = tdt_library::tdt_return_code::TDT_ERROR_NULL_PARAM;

        if (hagent != nullptr)
        {
            auto agentc_obj = get_agentc(hagent);
            if (agentc_obj == nullptr)
            {
                return TDT_ERROR_INVALID_PARAM;
            }
            std::string available_capabilities;
            ret_val = agentc_obj->discover(available_capabilities);
            if (ret_val == tdt_library::tdt_return_code::TDT_ERROR_SUCCESS ||
                ret_val == tdt_library::tdt_return_code::TDT_WARNING_NOT_ALL_PROFILES_LOADED)
            {
                if (caps_buffer_len <= (size_t)available_capabilities.length() ||
                    capabilities == nullptr)
                {
                    if (caps_len != nullptr)  // return expected buffer size
                    {
                        *caps_len = (size_t)(
                            available_capabilities.length() + 1);  // including null terminator
                    }
                    ret_val = tdt_library::tdt_return_code::TDT_ERROR_INSUFFICIENT_BUFFER_SIZE;
                }
                else
                {
                    memcpy_s(capabilities,
                        caps_buffer_len,
                        available_capabilities.data(),
                        available_capabilities.length());
                    capabilities[available_capabilities.length()] = 0;  // null terminate
                    if (caps_len != nullptr)
                    {
                        *caps_len = (size_t)(available_capabilities.length() + 1);
                    }
                }
            }
        }

        return static_cast<::tdt_return_code>(ret_val);
    }

    const ::tdt_return_code get_configuration(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len,
        char* configuration,
        const size_t config_buffer_len,
        size_t* config_len)
    {
        tdt_library::tdt_return_code ret_val = tdt_library::tdt_return_code::TDT_ERROR_NULL_PARAM;

        if (hagent != nullptr)
        {
            auto agentc_obj = get_agentc(hagent);
            if (agentc_obj == nullptr)
            {
                return TDT_ERROR_INVALID_PARAM;
            }
            std::string config;
            std::string profile;
            if (opt_profile_name == nullptr || opt_profile_name_len != 0)
            {
                if (opt_profile_name != nullptr)
                {
                    profile = std::string(opt_profile_name, opt_profile_name_len);
                }
                ret_val = agentc_obj->get_configuration(profile, config);
                if (ret_val == tdt_library::tdt_return_code::TDT_ERROR_SUCCESS ||
                    ret_val == tdt_library::tdt_return_code::TDT_WARNING_NOT_ALL_PROFILES_LOADED)
                {
                    if (config_buffer_len <= (size_t)config.length() || configuration == nullptr)
                    {
                        if (config_len != nullptr)  // return expected buffer size
                        {
                            *config_len =
                                (size_t)(config.length() + 1);  // including null terminator
                        }
                        ret_val = tdt_library::tdt_return_code::TDT_ERROR_INSUFFICIENT_BUFFER_SIZE;
                    }
                    else
                    {
                        memcpy_s(configuration, config_buffer_len, config.data(), config.length());
                        configuration[config.length()] = 0;  // null terminate
                        if (config_len != nullptr)
                        {
                            *config_len = (size_t)(config.length() + 1);
                        }
                    }
                }
            }
            else
            {
                ret_val = tdt_library::tdt_return_code::TDT_ERROR_INVALID_PARAM;
            }
        }

        return static_cast<::tdt_return_code>(ret_val);
    }

    const ::tdt_return_code set_configuration(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len,
        const char* opt_config,
        const size_t opt_config_buffer_len)
    {
        tdt_library::tdt_return_code ret_val = tdt_library::tdt_return_code::TDT_ERROR_NULL_PARAM;

        if (hagent != nullptr && ((opt_profile_name != nullptr && opt_profile_name_len != 0) ||
                                     (opt_config != nullptr && opt_config_buffer_len != 0)))
        {
            auto agentc_obj = get_agentc(hagent);
            if (agentc_obj == nullptr)
            {
                return TDT_ERROR_INVALID_PARAM;
            }
            std::string config;
            std::string profile;
            if (opt_profile_name != nullptr)
            {
                profile = std::string(opt_profile_name, opt_profile_name_len);
            }
            if (opt_config != nullptr)
            {
                config = std::string(opt_config, opt_config_buffer_len);
            }

            ret_val = agentc_obj->set_configuration(profile, config);
        }

        return static_cast<::tdt_return_code>(ret_val);
    }

    const ::tdt_return_code start(const agent_handle hagent)
    {
        if (hagent != nullptr)
        {
            auto agentc_obj = get_agentc(hagent);
            if (agentc_obj != nullptr)
            {
                return static_cast<::tdt_return_code>(agentc_obj->start());
            }
        }
        else
        {
            return TDT_ERROR_NULL_PARAM;
        }

        return TDT_ERROR_INVALID_PARAM;
    }

    const ::tdt_return_code stop(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len)
    {
        tdt_library::tdt_return_code ret_val = tdt_library::tdt_return_code::TDT_ERROR_NULL_PARAM;

        if (hagent != nullptr)
        {
            auto agentc_obj = get_agentc(hagent);
            if (agentc_obj == nullptr)
            {
                return TDT_ERROR_INVALID_PARAM;
            }
            if (opt_profile_name == nullptr || opt_profile_name_len != 0)
            {
                std::string profile;
                if (opt_profile_name != nullptr)
                {
                    profile = std::string(opt_profile_name, opt_profile_name_len);
                }
                ret_val = agentc_obj->stop(profile);
            }
            else
            {
                ret_val = tdt_library::tdt_return_code::TDT_ERROR_INVALID_PARAM;
            }
        }

        return static_cast<::tdt_return_code>(ret_val);
    }

    const ::tdt_return_code set_notification_callback(const agent_handle hagent,
        const notification_t callback,
        const long long context)
    {
        if (hagent != nullptr && callback != nullptr)
        {
            auto agentc_obj = get_agentc(hagent);
            if (agentc_obj != nullptr)
            {
                return static_cast<::tdt_return_code>(
                    agentc_obj->set_notification_callback(callback, context));
            }
        }
        else
        {
            return TDT_ERROR_NULL_PARAM;
        }

        return TDT_ERROR_INVALID_PARAM;
    }

    const char* get_error_string(const ::tdt_return_code code)
    {
        return tdt_library::agent::get_error_string(
            static_cast<tdt_library::tdt_return_code>(code));
    }

    const agent_handle construct_agent(const tdt_protocol_format proto_fmt)
    {
        try
        {
            auto agentc_obj =
                std::make_shared<agentc>(static_cast<tdt_library::tdt_protocol_format>(proto_fmt));
            if (agentc_obj != nullptr)
            {
                auto handle = get_handle(agentc_obj);
                if (handle != 0)
                {
                    return reinterpret_cast<agent_handle>(handle);
                }
                else
                {
                    agentc_obj = nullptr;
                }
            }
        }
        catch (const std::bad_alloc&)
        {}
        catch (const std::invalid_argument&)
        {}

        return nullptr;
    }

    const ::tdt_return_code destruct_agent(const agent_handle hagent)
    {
        if (hagent != nullptr)
        {
            if (remove_handle(hagent))
            {
                return TDT_ERROR_SUCCESS;
            }
        }
        else
        {
            return TDT_ERROR_NULL_PARAM;
        }

        return TDT_ERROR_INVALID_PARAM;
    }

}  // extern "C"
