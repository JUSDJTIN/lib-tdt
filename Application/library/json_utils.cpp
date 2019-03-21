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
 **    @file   json_utils.cpp
 **
 **    @brief  Implements JSON utility functions.
 **
 **
 ********************************************************************************
 */

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include "json_utils.h"

namespace json_utils
{
    /**
     * @brief Validates if a string represents a number.
     *
     * @param[in] strnum the string to test if it represents a number.
     * @return returns true if the string is a number; false otherwise.
     */
    static bool is_number(const std::string& strnum)
    {
        bool is_a_number = false;
        try
        {
            boost::lexical_cast<double>(strnum);
            is_a_number = true;
        }
        catch (const boost::bad_lexical_cast&)
        {
            // it's not a number!
        }
        return is_a_number;
    }

    void output_json(const boost::property_tree::ptree& tree, std::stringstream& ss)
    {
        if (tree.empty())
        {  // property value
            std::string value = tree.get_value("");
            boost::algorithm::trim(value);
            if (value.empty())
            {
                ss << "null";
            }
            else if (value != "true" && value != "false" && !is_number(value))
            {                          // output as a string
                if (value.find('\\'))  // escape if string contains path separators(windows)
                {
                    boost::replace_all(value, "\\", "\\\\");
                }
                ss << "\"" << value << "\"";
            }
            else
            {  // output as boolean or number
                ss << value;
            }
        }
        else
        {
            size_t num_objs = tree.size();
            for (const auto& child : tree)  // iterate through list of children
            {
                --num_objs;
                if (!child.first.empty())
                {  // property key
                    ss << "\"" << child.first << "\":";
                }
                if (!child.second.empty())
                {  // begining of property which is an object or an array
                    if (child.second.front().first.empty())
                    {  // output as an array if key is empty
                        ss << "[";
                    }
                    else
                    {  // output as an object
                        ss << "{";
                    }
                }
                output_json(child.second, ss);  // recursively iterate each child
                if (!child.second.empty())
                {  // end of property which is an object or an array
                    if (child.second.front().first.empty())
                    {
                        ss << "]";
                    }
                    else
                    {
                        ss << "}";
                    }
                    if (num_objs > 0)
                    {
                        ss << ",";
                    }  // else avoid adding trailing , in an object containing more than 1 members.
                }
                else
                {  // end of an object enclosed in another object and begining of another property
                   // in the enclosing object
                    if (num_objs > 0)
                    {
                        ss << ",";
                    }  // else avoid adding trailing , in an object containing more than 1 members.
                }
            }
        }
    }
}  // namespace json_utils