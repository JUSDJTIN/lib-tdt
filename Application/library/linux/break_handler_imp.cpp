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
**    @file   break_handler_imp.cpp
**
**    @brief  Notifies subscriber of a Ctrl+C event
**
**
********************************************************************************
*/
#include <unistd.h>
#include "break_handler_imp.h"

namespace tdt_utility
{
    //----------------------------------------------------------------------------
    break_handler_imp::break_handler_imp()
        : m_callback{nullptr}
    {
        struct sigaction handler;
        handler.sa_handler = ctrl_handler;
        sigemptyset(&handler.sa_mask);
        handler.sa_flags = 0;
        sigaction(SIGINT, &handler, &m_old_handler);
    }

    //----------------------------------------------------------------------------
    break_handler_imp::~break_handler_imp()
    {
        sigaction(SIGINT, &m_old_handler, NULL);
    }

    //----------------------------------------------------------------------------
    void break_handler_imp::register_handler(const break_handler_callback_t& callback)
    {
        if (callback)
        {
            m_callback = callback;
        }
    }

    //----------------------------------------------------------------------------
    void break_handler_imp::invoke_callback() const
    {
        if (m_callback)
        {
            m_callback();
        }
    }

    //----------------------------------------------------------------------------
    void break_handler_imp::ctrl_handler(int s)
    {
        const auto& instance = break_handler_imp::instance();
        instance.invoke_callback();
    }
}  // namespace tdt_utility
