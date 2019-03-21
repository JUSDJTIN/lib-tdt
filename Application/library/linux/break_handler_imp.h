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
**    @file   break_handler_imp.h
**
**    @brief  Notifies subscriber of a Ctrl+c event
**
**
********************************************************************************
*/
#ifndef BREAK_HANDLER_IMP_H
#define BREAK_HANDLER_IMP_H

#include <signal.h>
#include "break_handler.h"

namespace tdt_utility
{
    class break_handler_imp : public break_handler
    {
    public:
        /**
         * Get an instance of break_handler_imp
         * @return an instance of this class
         */
        static break_handler_imp& instance()
        {
            static break_handler_imp instance;
            return instance;
        };

        /**
         * @brief Register as a break Handler
         * @param callback   The callback to invoke when Ctrl+c is captured
         */
        void register_handler(const break_handler_callback_t& callback) override;

        /**
         * @brief Invoke callback registered
         */
        void invoke_callback() const;

        /**
         * @brief class destructor;
         */
        virtual ~break_handler_imp();

    private:
        /**
         * @brief Class constructor
         */
        break_handler_imp();

        /**
         * @brief Copy and assignments operators
         */
        break_handler_imp(const break_handler_imp&) = delete;
        break_handler_imp& operator=(break_handler_imp const&) = delete;

        /**
         * @brief Method to register as a Ctrl Handler that will
         * call the function registered
         */
        static void ctrl_handler(int s);

        // Member Variable
        break_handler_callback_t m_callback;
        struct sigaction m_old_handler;
    };
}  // namespace tdt_utility
#endif  // BREAK_HANDLER_H
