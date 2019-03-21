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