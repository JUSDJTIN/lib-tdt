/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
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