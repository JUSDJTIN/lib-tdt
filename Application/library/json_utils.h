/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   json_utils.h
**
**    @brief  Defines JSON utility functions.
**
**
********************************************************************************
*/
#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <sstream>
#include <boost/property_tree/info_parser.hpp>

namespace json_utils
{
    /**
     * @brief Outputs property tree in JSON object format.
     *
     * @param[in] tree the tree to output in JSON.
     * @param[in,out] ss the stream to write the JSON output.
     */
    void output_json(const boost::property_tree::ptree& tree, std::stringstream& ss);
}  // namespace json_utils

#endif  // JSON_UTILS_H