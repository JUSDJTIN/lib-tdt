/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   os_name.h
**
**    @brief  Obtain a constant string representing the current OS
**
**
********************************************************************************
*/

#ifndef TDT_OS_NAME_H
#define TDT_OS_NAME_H

#include <string>

#if defined(_WIN32)
const std::string OS_NAME = "windows";
#elif defined(__linux__)
const std::string OS_NAME = "linux";
#else
const std::string OS_NAME = "unknown";
#endif  // defined(_WIN32)

#endif  // TDT_OS_NAME_H