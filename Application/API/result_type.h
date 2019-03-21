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
**    @file   result_type.h
**
**    @brief  Result and error reporting type for bit shovel
**
**
********************************************************************************
*/

#ifndef BIT_SHOVEL_RESULT_TYPE_H
#define BIT_SHOVEL_RESULT_TYPE_H

#include <sstream>
#include <memory>

namespace bit_shovel
{

    namespace internal
    {
        // Passing nullptr into a output stream causes
        // it to be initialized with a nullptr stream_buf.
        // A stream with a nullptr stream_buf is
        // in an invalid state causing it to ignore input.
        static std::ostream g_result_type_null_stream(nullptr);
    }  // namespace internal

    /**
     * @brief Helper class to aid in status and error reporting.
     * @remarks The class represents success by default and changes to evaluate as failure
     *          if a string description of the failure is added via the 'add_failure()' method.
     */
    class result_type
    {
    public:
        /**
         * @brief Class constructor
         */
        result_type()
            : m_stream()
        {}

        /**
         * @brief Class Copy constructor
         */
        result_type(const result_type& other)
        {
            if (other.m_stream != nullptr)
            {
                if (m_stream == nullptr)
                {
                    m_stream = std::unique_ptr<typename decltype(m_stream)::element_type>{
                        new decltype(m_stream)::element_type()};
                }
                *m_stream << other.m_stream->rdbuf();
            }
        }

        /**
         * @brief move Copy constructor
         */
        result_type(result_type&& other)
            : m_stream(std::move(other.m_stream))
        {}

        /**
         * @brief Check if this result_type represents an error
         * @return returns true if this result_type represents an error
         */
        bool errored() const
        {
            return m_stream != nullptr;
        }

        /**
         * @brief Check if this result_type represents a success
         * @ return returns true if this result_type represents a success
         */
        bool succeeded() const
        {
            return !this->errored();
        }

        /**
         * @brief Obtain a human-readable string describing a failure.
         * @return If this result_type is in a failure state, returns string (possibly multi-line)
         * that describes the failure. If this result_type is in a success state, returns empty
         * string.
         */
        std::string what() const
        {
            if (m_stream != nullptr) return m_stream->str();
            return "";
        }

        /**
         * @brief Check if this result_type represents a success
         * @return returns true if the result_type hold represents a success
         */
        operator bool()
        {
            return !this->errored();
        }

        /**
         * @brief Assignment operator
         */
        result_type& operator=(const result_type& other)
        {
            if (other.m_stream != nullptr)
            {
                if (this->m_stream == nullptr)
                {
                    this->m_stream = std::unique_ptr<typename decltype(m_stream)::element_type>{
                        new decltype(m_stream)::element_type()};
                }
                *(this->m_stream) << other.m_stream->rdbuf();
            }
            return *this;
        }

        /**
         * @brief move assignment operator
         */
        result_type& operator=(result_type&& other)
        {
            this->m_stream = std::move(other.m_stream);
            return *this;
        }

        /**
         * @brief Mark this result_type as a failure and return a output stream to add details
         * @return returns an output stream to add failure details
         * @remark typical usage would be result.add_failure() << "error message." << std::endl;
         */
        std::ostream& add_failure()
        {
            if (m_stream == nullptr)
            {
                m_stream = std::unique_ptr<typename decltype(m_stream)::element_type>{
                    new decltype(m_stream)::element_type()};
                *m_stream << "An error occurred during execution:" << std::endl;
            }

            return *m_stream.get();
        }

        /**
        * @brief Return a output stream to add more levels of details
        *        but only if the result_type is already a failure
        * @return Returns an output stream to add failure details if already a failure.
                  Otherwise, returns an invalid stream which will ignore input.
        * @remark typical usage would be similar to
        *         result.add_if_failure() << "plugin " << plugin_name << " failed to load." <<
        std::endl;
        */
        std::ostream& add_if_failure()
        {
            if (this->errored())
            {
                return *m_stream.get();
            }
            else
            {
                return internal::g_result_type_null_stream;
            }
        }

    private:
        // private member to help with construction/storage of failure descriptions.
        std::unique_ptr<std::stringstream> m_stream;
    };

}  // namespace bit_shovel

#endif  // BIT_SHOVEL_RESULT_TYPE_H
