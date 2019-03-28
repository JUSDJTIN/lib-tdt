/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   sign_handler_interface.h
**
**    @brief  Interface to invoke signature verification code
**
**
********************************************************************************
*/
#ifndef SIGN_HANDLER_INTERFACE_H
#define SIGN_HANDLER_INTERFACE_H

namespace tdt_utility
{
    class sign_handler_interface
    {
    public:
        /**
         * @brief Check if library contains a valid signature
         * @param dll_path Path to the dll to be checked
         */
        virtual bool library_is_valid(const boost::filesystem::path& dll_path) = 0;

        /**
         * @brief class destructor;
         */
        virtual ~sign_handler_interface() = default;
    };
}  // namespace tdt_utility
#endif  // SIGN_HANDLER_INTERFACE_H