/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   tdt_agent.h
**
**    @brief  Defines the C entry point for ThreatDetection Technology Library.
**
**
********************************************************************************
*/
#include <stddef.h>

#ifndef TDT_AGENT_H
#    define TDT_AGENT_H

#    if defined(_WIN32)
#        ifdef DLL_EXPORTS
#            define TDT_API_EXPORT __declspec(dllexport)
#        else
#            define TDT_API_EXPORT __declspec(dllimport)
#        endif
#    else
#        define TDT_API_EXPORT
#    endif

#    define VALID_HANDLE(h) ((h) != ((void*)0))
#    define INVALID_HANDLE(h) ((h) == ((void*)0))

#    ifdef __cplusplus
extern "C"
{
#    endif
    /**
     * @brief Return codes from agent APIs.
     *
     */
    typedef enum tdt_return_code_
    {
        TDT_ERROR_SUCCESS,
        TDT_ERROR_NULL_PARAM,
        TDT_ERROR_INVALID_PARAM,
        TDT_ERROR_OUT_OF_MEMORY,
        TDT_ERROR_INTERNAL,
        TDT_ERROR_INSUFFICIENT_BUFFER_SIZE,
        TDT_ERROR_NOT_IMPLEMENTED,
        TDT_ERROR_STARTUP_FAILURE,
        TDT_ERROR_INVALID_PLUGIN,
        TDT_ERROR_INVALID_CONFIG,
        TDT_ERROR_NO_EXECUTION,
        TDT_ERROR_AGENT_RUNNING,
        TDT_ERROR_AGENT_NOT_RUNNING,
        TDT_ERROR_AGENT_ABORTED,
        TDT_ERROR_SIGNVERIFY_FAILED,
        TDT_ERROR_NO_PROFILES_AVAILABLE,
        TDT_WARNING_NOT_ALL_PROFILES_LOADED,
        TDT_ERROR_PROFILES_DIR_NOT_EXISTS,
        TDT_ERROR_AGENT_UNABLE_TO_STOP,
        TDT_ERROR_PIPELINE_NOT_FUNCTIONAL,

        TDT_ERROR_MAX
    } tdt_return_code;

    /**
     * @brief Communication protocol formats supported by the library.
     *
     */
    typedef enum tdt_protocol_format_
    {
        TDT_PROTO_FORMAT_JSON,
        TDT_PROTO_FORMAT_XML,

        TDT_PROTO_FORMAT_MAX
    } tdt_protocol_format;

    /**
     * @brief The tdt agent opaque class.
     *
     */
    typedef struct agentc* agent_handle;

    /**
     * @brief Constructor
     *
     * @param[in] proto_fmt Protocol format the user wishes to use.
     *
     * @return valid handle on success or NULL on failure.
     */
    TDT_API_EXPORT const agent_handle construct_agent(const tdt_protocol_format proto_fmt);
    typedef const agent_handle (*construct_agent_t)(const tdt_protocol_format proto_fmt);

    /**
     * @brief Destructor
     *
     * @param[in] hagent the agent handle returned by construct_agent.
     */
    TDT_API_EXPORT const tdt_return_code destruct_agent(const agent_handle hagent);
    typedef const tdt_return_code (*destruct_agent_t)(const agent_handle hagent);

    /**
     * @brief The notification callback signature.
     *
     * @param[in] context the context that was passed to set_notification_callback.
     * @param[in] msg the notification message.
     * @param[in] msg_len the length of msg in bytes.
     */
    typedef void (*notification_t)(const long long context, const char* msg, const size_t msg_len);

    /**
     * @brief discover version, build and supported profiles.
     *
     * @param[in] hagent the agent handle returned by construct_agent.
     * @param[out] capabilities on return from this API it will contain a null terminated JSON
     * object containing version, build info, available profiles with properties.
     * @code{.json} output: {"version": "1.2.1", "build":{"date":"Feb 18 2019", time:
     * "23:59:01"},"profiles":
     *                       [{"rfc_ml_sc":{"description":"side channel","state":"active"}},
     *                        {"rfc_ml_cj":{"description":"crypto mining","state":"inactive"}}]}
     * @endcode
     * @param[in] caps_buffer_len size of the capabilities buffer in bytes including null
     * terminator.
     * @param[out] caps_len length in bytes of the capabilities including null terminator populated
     * in the buffer if capabilities is NULL then caps_len if not NULL will contain the size of the
     * buffer required to populate the capabilities including the null terminator.
     *
     * @return  TDT_ERROR_SUCCESS on success or an error code.
     *          if TDT_WARNING_NOT_ALL_PROFILES_LOADED is returned then partial list of discovered
     * profiles is returned. even on a error code minimum versiona and build information will be
     * returned in capabilities.
     */
    TDT_API_EXPORT const tdt_return_code discover(const agent_handle hagent,
        char* capabilities,
        const size_t caps_buffer_len,
        size_t* caps_len);
    typedef const tdt_return_code (*discover_t)(const agent_handle hagent,
        char* capabilities,
        size_t caps_buffer_len,
        size_t* caps_len);

    /**
     * @brief get current supported configurations for all profiles or a specific profile.
     *
     * @param[in] hagent the agent handle returned by construct_agent.
     * @param[in] opt_profile_name optional name of the profile for which to get the configuration.
     * can be NULL.
     * @param[in] opt_profile_name_len length of opt_profile_name. ignored if opt_profile_name is
     * NULL.
     * @param[out] config on return will contain a JSON object describing current configurations for
     * profile(s).
     * @param[in] config_buffer_len the size of the config buffer in bytes.
     * @param[out] config_len the length in bytes of configurations populated in config.
     * @code{.json} input: opt_profile: "rfc_ml_cj"
     * @endcode
     * @code{.json} output: gconfig: {"configurations" : [{"rfc_ml_cj":{ "normalizer": {"model":
     * {"t0_features_per_tid": false}}}}]}
     * @endcode
     * @code{.json} input: opt_profile: ""
     * @endcode
     * @code{.json} output: gconfig: {"configurations" : [{"rfc_ml_sc":{ "max_detections": 1200}},
     * {"rfc_ml_cj":{ "report_rate": 500}}]}
     * @endcode
     *
     * @return  TDT_ERROR_SUCCESS on success or an error code.
     *          if TDT_WARNING_NOT_ALL_PROFILES_LOADED is returned then partial list of profiles
     * with their configurations is returned.
     */
    TDT_API_EXPORT const tdt_return_code get_configuration(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len,
        char* config,
        const size_t config_buffer_len,
        size_t* config_len);
    typedef const tdt_return_code (*get_configuration_t)(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len,
        char* config,
        const size_t config_buffer_len,
        size_t* config_len);

    /**
     * @brief set configurations for profiles or a specific profile that need to be started when
     * start is called.
     *
     * @param[in] hagent the agent handle returned by construct_agent.
     * @param[in] opt_profile_name optional name of the profile for which to set the configuration.
     * can be NULL. if NULL then opt_config object should contain profile name with the
     * configuration properties to set.
     * @param[in] opt_profile_name_len length of opt_profile_name if opt_profile_name is not NULL.
     * @param[in] opt_config a JSON object to modify current configuration for the profile.
     *               if NULL then opt_profile can't be NULL and current/default configuration will
     * be applied for profile specified in opt_profile.
     * @param[in] opt_config_buffer_len the size of the config buffer in bytes if opt_config is not
     * NULL.
     * @code{.json} input: opt_profile: ""
     * @endcode
     * @code{.json} input: sconfig: {"configurations" : [{"rfc_ml_sc":{ "max_detections": 1200}},
     * {"rfc_ml_cj":{ "report_rate": 500}}]} input: sconfig: {{"rfc_ml_sc":{ "max_detections":
     * 1200}}, {"rfc_ml_cj":{ "report_rate": 500}}}
     * @endcode
     * @code{.json} input: opt_profile: "rfc_ml_cj"
     * @endcode
     * @code{.json} input: sconfig: {"normalizer": {"model": {"t0_features_per_tid": true}}}
     * input: sconfig: {"rfc_ml_sc":{ "normalizer": {"model": {"t0_features_per_tid": true}}}}
     * @endcode
     *
     * @return  TDT_ERROR_SUCCESS on success or an error code. On success existing profiles
     * configurations will be cleared and the new ones will be set to run after calling start.
     *
     */
    TDT_API_EXPORT const tdt_return_code set_configuration(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len,
        const char* opt_config,
        const size_t opt_config_buffer_len);
    typedef const tdt_return_code (*set_configuration_t)(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len,
        const char* opt_config,
        const size_t opt_config_buffer_len);

    /**
     * @brief Applies profiles set by set_configuration and starts detection process.
     *
     * @param[in] hagent the agent handle returned by construct_agent.
     *
     * @return  TDT_ERROR_SUCCESS on success or an error code.
     */
    TDT_API_EXPORT const tdt_return_code start(const agent_handle hagent);
    typedef const tdt_return_code (*start_t)(const agent_handle hagent);

    /**
     * @brief Stops current detection process for specified profile or all profiles.
     *
     * @param[in] hagent the agent handle returned by construct_agent.
     * @param[in] opt_profile_name optional name of the profile to stop detection. can be NULL.
     * @param[in] opt_profile_name_len length of opt_profile_name. ignored if opt_profile_name is
     * NULL.
     *
     * @return  TDT_ERROR_SUCCESS on success or an error code.
     */
    TDT_API_EXPORT const tdt_return_code stop(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len);
    typedef const tdt_return_code (*stop_t)(const agent_handle hagent,
        const char* opt_profile_name,
        const size_t opt_profile_name_len);

    /**
     * @brief set callback for notifications.
     *
     * @param[in] hagent the agent handle returned by construct_agent.
     * @param[in] callback to send notifications.
     * @param[in] context to send with notifications. cannot be 0.
     *
     * @return  TDT_ERROR_SUCCESS on success or an error code.
     */
    TDT_API_EXPORT const tdt_return_code set_notification_callback(const agent_handle hagent,
        const notification_t callback,
        const long long context);
    typedef const tdt_return_code (*set_notification_callback_t)(const agent_handle hagent,
        const notification_t callback,
        const long long context);

    /**
     * @brief get string describing an error code.
     *
     * @param[in] code error code returned by the APIs.
     * @code{.c} output: "out of memory"
     * @endcode
     *
     * @return  a pointer to a null terminated string describing the error code.
     *          returns NULL for invalid error codes.
     */
    TDT_API_EXPORT const char* get_error_string(const tdt_return_code code);
    typedef const char* (*get_error_string_t)(const tdt_return_code code);

#    ifdef __cplusplus
}  // extern "C"
#    endif

#endif /* TDT_AGENT_H */