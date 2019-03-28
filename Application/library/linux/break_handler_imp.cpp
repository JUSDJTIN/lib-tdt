/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
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
