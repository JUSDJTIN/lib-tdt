/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   sign_handler_imp.cpp
**
**    @brief  verifies the library authenticity being loaded
**
**
********************************************************************************
*/

#include "sign_handler_imp.h"

namespace tdt_utility
{
    //----------------------------------------------------------------------------
    sign_handler_imp& sign_handler_imp::instance()
    {
        static sign_handler_imp instance;
        return instance;
    };

    //----------------------------------------------------------------------------
    bool sign_handler_imp::library_is_valid(const boost::filesystem::path& dll_path)
    {
        if (boost::filesystem::exists(dll_path))
        {
            return true;
        }
        return false;
    }
}  // namespace tdt_utility