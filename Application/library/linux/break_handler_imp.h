/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
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
