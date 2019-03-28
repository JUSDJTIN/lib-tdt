/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   sign_handler_imp.h
**
**    @brief  verifies the library authenticity being loaded
**
**
********************************************************************************
*/
#ifndef SIGN_HANDLER_IMPL_H
#define SIGN_HANDLER_IMPL_H

#include <boost/filesystem/operations.hpp>

#include "sign_handler_interface.h"

namespace tdt_utility
{
    class sign_handler_imp : public sign_handler_interface
    {
    public:
        /**
         * @brief Copy and assignments operators
         */
        sign_handler_imp(const sign_handler_imp&) = delete;
        sign_handler_imp& operator=(sign_handler_imp const&) = delete;

        /**
         * Create only single instance of sign_handler_imp - Singleton Class
         * @return an instance of this class
         */
        static sign_handler_imp& instance();

        /**
         * @brief Check if library contains a valid signature
         * @param dll_path Path to the dll to be checked
         */
        bool library_is_valid(const boost::filesystem::path& dll_path) override;

    private:
        /**
         * @brief Class constructor
         */
        sign_handler_imp() = default;
    };
}  // namespace tdt_utility
#endif  // ! SIGN_HANDLER_IMPL_H