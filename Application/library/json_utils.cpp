/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
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