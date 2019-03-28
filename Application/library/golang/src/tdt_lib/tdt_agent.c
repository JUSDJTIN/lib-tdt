/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   tdt_agent.c
**
**    @brief  CGO notification interface for Threat Detection Technology Library.
**
**
********************************************************************************
*/

#include "tdt_agent.h"

/** @brief Generated at run time by the cgo module.
 *          Exports the GoNotifier from Go to be accessed from C
 */
#include "_cgo_export.h"

/**
 * @brief The notification bridge function between Go and C languages.
 *
 * @param[in] context the context that was passed to SetNotificationCallback
 * @param[in] msg the notification message
 * @param[in] msg_len the length of msg in bytes
 */
void NotifierBridgeCallback(const long long context, const char* msg, const size_t msg_len)
{
    GoNotifier(context, (GoString){msg, msg_len});
}

/**
 * @brief sets the intermediate bridge callback for notifications
 *
 * @param[in] hagent the agent handle returned by construct_agent
 * @param[in] context to send with notifications
 *
 * @return  TDT_ERROR_SUCCESS on success or an error code
 */
tdt_return_code SetNotificationCallback(agent_handle hagent, const long long context)
{
    return set_notification_callback(hagent, NotifierBridgeCallback, context);
}