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
**    @file   data_source_base.h
**
**    @brief  Base class for data source life-cycle control APIs used by the pipeline
**
**
********************************************************************************
*/

#ifndef BIT_SHOVEL_DATA_SOURCE_BASE_H
#define BIT_SHOVEL_DATA_SOURCE_BASE_H

#include "result_type.h"

namespace bit_shovel
{

    /**
     * @brief Base class for data source life-cycle control APIs used by the pipeline
     * @remark This serves a generic API for the pipeline to begin and end external data collect by
     * plugins
     */
    class data_source_base
    {
    public:
        /**
         * @brief Virtual destructor to ensure proper derived class cleanup
         */
        virtual ~data_source_base() = default;

        /**
         * @brief Method which instructs the data source to begin pushing data into the graph
         */
        virtual result_type start() = 0;

        /**
         * @brief Method which instructs the data source to stop pushing data into the graph
         */
        virtual result_type stop() = 0;
    };

}  // namespace bit_shovel

#endif  // BIT_SHOVEL_DATA_SOURCE_BASE_H