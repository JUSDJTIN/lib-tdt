/*
********************************************************************************
**    Intel Architecture Group
**    Copyright (C) 2018 Intel Corporation
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