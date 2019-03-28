/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   channel_registry.cpp
**
**    @brief  Class to keep track of which plugins have registered source
**            or sink capabilities during the registration phase.
**
********************************************************************************
*/
#include "channel_registry.h"

namespace bit_shovel
{

    channel_registry::channel_registry()
        : m_private(std::make_shared<decltype(m_private)::element_type>())
    {}

}  // namespace bit_shovel
