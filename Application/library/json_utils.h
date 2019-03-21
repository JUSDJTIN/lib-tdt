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