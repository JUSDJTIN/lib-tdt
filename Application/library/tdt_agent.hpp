/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   tdt_agent.hpp
**
**    @brief  Defines the C++ entry point for ThreatDetection Technology Library.
**
**
********************************************************************************
*/

#ifndef TDT_AGENT_HPP
#define TDT_AGENT_HPP

#include <string>
#include <memory>
#include <functional>

#if defined(_WIN32)
#    pragma warning(disable : 4251)  // class needs to have a dll interface
#    ifdef DLL_EXPORTS
#        define TDT_API_EXPORT __declspec(dllexport)
#    else
#        define TDT_API_EXPORT __declspec(dllimport)
#    endif
#else
#    define TDT_API_EXPORT
#endif

namespace tdt_library
{
    /**
     * @brief The notification callback signature.
     *
     * @param[in] context the context that was passed to set_notification_callback.
     * @param[in] msg the notification message.
     */
    using notification_t = std::function<void(const long long context, const std::string& msg)>;

    /**
     * @brief Return codes from agent APIs.
     * @note Update in tdt_agent.h/.go if updated.
     */
    enum tdt_return_code : uint32_t
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
    };

    /**
     * @brief Communication protocol formats supported by the library.
     *
     */
    enum tdt_protocol_format : uint32_t
    {
        TDT_PROTO_FORMAT_JSON,
        TDT_PROTO_FORMAT_XML,

        TDT_PROTO_FORMAT_MAX
    };

    // forward declaration
    class tdt_agent_impl;

    /**
     * @brief The tdt agent class.
     *
     * The interface to configure various threat profiles and detect different threats.
     */
    class TDT_API_EXPORT agent
    {
    public:
        /**
         * @brief Constructor.
         *
         */
        agent();

        /**
         * @brief Constructor.
         *
         * @param[in] proto_fmt Protocol format the user wishes to use. JSON is default if proto_fmt
         * is not specified.
         * @throw std::invalid_argument if proto_fmt is invalid.
         * @throw other exceptions like std::bad_alloc.
         */
        agent(tdt_protocol_format proto_fmt);

        /**
         * @brief Destructor.
         */
        ~agent();

        // No Copy constructor or copy assignment
        agent(const agent& orig) = delete;
        agent& operator=(const agent&) = delete;

        /**
         * @brief discover version, build and supported profiles.
         *
         * @param[out] capabilities on return from this API it will contain a JSON object
         *                          containing version, build, available profiles with properties.
         * @code{.json} output: {"version": "1.2.1", "build":{"date":"Feb 18 2019", time:
         * "23:59:01"},"profiles":
         *                       [{"rfc_ml_sc":{"description":"side channel","state":"active"}},
         *                        {"rfc_ml_cj":{"description":"crypto mining","state":"inactive"}}]}
         * @endcode
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         *          if TDT_WARNING_NOT_ALL_PROFILES_LOADED is returned then partial list of
         * discovered profiles is returned. even on a error code minimum versiona and build
         * information will be returned in capabilities.
         */
        const tdt_return_code discover(std::string& capabilities);

        /**
         * @brief get current supported configurations for all profiles or a specific profile.
         *
         * @param[in] opt_profile optional name of profile for which to get the configuration. this
         * can be empty.
         * @param[out] gconfig on return will contain a JSON object describing current
         * configurations for profile(s).
         * @code{.json} input: opt_profile: "rfc_ml_cj"
         * @endcode
         * @code{.json} output: gconfig: {"configurations" : [{"rfc_ml_cj":{ "normalizer": {"model":
         * {"t0_features_per_tid": false}}}}]}
         * @endcode
         * @code{.json} input: opt_profile: ""
         * @endcode
         * @code{.json} output: gconfig: {"configurations" : [{"rfc_ml_sc":{ "max_detections":
         * 1200}}, {"rfc_ml_cj":{ "report_rate": 500}}]}
         * @endcode
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         *          if TDT_WARNING_NOT_ALL_PROFILES_LOADED is returned then partial list of profiles
         * with their configurations is returned.
         */
        const tdt_return_code get_configuration(const std::string& opt_profile,
            std::string& gconfig);

        /**
         * @brief set configurations for profiles or a specific profile that need to be started when
         * start is called.
         *
         * @param[in] opt_profile optional name of a specific profile for which to set the
         * configuration. if empty then sconfig object should contain profile name with the
         * configuration properties to set.
         * @param[in] sconfig a JSON object to modify current configuration for the profile.
         *               if empty then opt_profile can't be empty and current/default configuration
         * will be applied for profile specified in opt_profile.
         * @code{.json} input: opt_profile: ""
         * @endcode
         * @code{.json}  input: sconfig: {"configurations" : [{"rfc_ml_sc":{ "max_detections":
         * 1200}}, {"rfc_ml_cj":{ "report_rate": 500}}]} input: sconfig: {{"rfc_ml_sc":{
         * "max_detections": 1200}}, {"rfc_ml_cj":{ "report_rate": 500}}}
         * @endcode
         * @code{.json} input: opt_profile: "rfc_ml_cj"
         * @endcode
         * @code{.json}  input: sconfig: {"normalizer": {"model": {"t0_features_per_tid": true}}}
         * input: sconfig: {"rfc_ml_sc":{ "normalizer": {"model": {"t0_features_per_tid": true}}}}
         * @endcode
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code. On success existing profiles
         * configurations will be cleared and the new ones will be set to run after calling start.
         */
        const tdt_return_code set_configuration(const std::string& opt_profile,
            const std::string& sconfig);

        /**
         * @brief Applies profiles set by set_configuration and starts detection process.
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code start();

        /**
         * @brief Stops current detection process for specified profile or all profiles.
         * @param[in] opt_profile optional name of profile for which to stop detections.
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code stop(const std::string& opt_profile);

        /**
         * @brief set callback for notifications.
         *
         * @param[in] callback to send notifications.
         * @param[in] context to send with notifications. cannot be 0.
         *
         * @return  TDT_ERROR_SUCCESS on success or an error code.
         */
        const tdt_return_code set_notification_callback(notification_t callback,
            const long long context);

        /**
         * @brief get string describing an error code.
         *
         * @param[in] code an error code.
         * @code{.c} output: "out of memory"
         * @endcode
         *
         * @return  null terminated string describing the error code for valid error codes.
         *          returns nullptr for invalid error codes.
         *
         */
        static const char* get_error_string(const tdt_return_code code);

    private:
        std::unique_ptr<tdt_agent_impl> m_private;
    };

}  // namespace tdt_library

#endif /* TDT_AGENT_HPP */