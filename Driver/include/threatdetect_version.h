/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   threatdetect_version.h
**
********************************************************************************
*/


#ifndef _THREATDETECT_VERSION_H_
#define _THREATDETECT_VERSION_H_

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
#define _STRINGIFY_W(x) L#x
#define STRINGIFY_W(x) _STRINGIFY_W(x)

#define TD_MAJOR_VERSION 0
#define TD_MINOR_VERSION 1
#define TD_UPDATE_VERSION 0
#define TD_API_VERSION TD_UPDATE_VERSION
#if TD_UPDATE_VERSION > 0
#    define TD_UPDATE_STRING " Update " STRINGIFY(SEP_UPDATE_VERSION)
#else
#    define TD_UPDATE_STRING ""
#endif
#define TD_RELEASE_STRING "Dev"

#if defined(BUILD_INTERNAL)
#    define PRODUCT_TYPE "private"
#else
#    define PRODUCT_TYPE "public"
#endif

#if !defined(PRODUCT_BUILDER)
#    define PRODUCT_BUILDER unknown
#endif

#define TD_NAME "threatdetect"
#define TD_NAME_W L"threatdetect"

#define TD_PRODUCT_NAME "Threat Detection SideChannel Product"

#define TD_PRODUCT_VERSION_DATE __DATE__ " at " __TIME__

#define TD_PRODUCT_COPYRIGHT "Copyright(C) 2018 Intel Corporation. All rights reserved."

#define PRODUCT_DISCLAIMER                                                                       \
    "Warning: This computer program is protected under U.S. and international\ncopyright laws, " \
    "and may only be used or copied in accordance with the terms\nof the license agreement.  "   \
    "Except as permitted by such license, no part\nof this computer program may be reproduced, " \
    "stored in a retrieval system,\nor transmitted in any form or by any means without the "     \
    "express written consent\nof Intel Corporation."
#define TD_PRODUCT_VERSION "0.1"

#define TD_MSG_PREFIX TD_NAME "" STRINGIFY(TD_MAJOR_VERSION) "_" STRINGIFY(TD_MINOR_VERSION) ":"
#define TD_VERSION_STR \
    STRINGIFY(TD_MAJOR_VERSION) "." STRINGIFY(TD_MINOR_VERSION) "." STRINGIFY(TD_API_VERSION)

#if defined(DRV_OS_WINDOWS)

// TODO: Change this to use TD_NAME?
#    define TD_DRIVER_NAME "IntelTM"
#    define TD_DRIVER_NAME_W L"IntelTM"
#    define TD_DEVICE_NAME TD_DRIVER_NAME

#endif

#if defined(DRV_OS_LINUX) || defined(DRV_OS_SOLARIS) || defined(DRV_OS_ANDROID) || \
    defined(DRV_OS_FREEBSD)

#    define TD_DRIVER_NAME TD_NAME "-" STRINGIFY(TD_MAJOR_VERSION) "_" STRINGIFY(TD_MINOR_VERSION)
#    define TD_DEVICE_NAME "/dev/" TD_DRIVER_NAME
#    define TD_CONTROL_NODE TD_DEVICE_NAME "/control"
#    define TD_PROCESS_DATA_NODE TD_DEVICE_NAME "/sample"
#    define TD_LBR_DATA_NODE TD_DEVICE_NAME "/lbrdata"

#endif

#if defined(DRV_OS_MAC)
// TODO update this!
#    define TD_DRIVER_NAME TD_NAME "" STRINGIFY(TD_MAJOR_VERSION) "_" STRINGIFY(TD_MINOR_VERSION)
#    define TD_SAMPLES_NAME TD_DRIVER_NAME "_s"
#    define TD_DEVICE_NAME TD_DRIVER_NAME

#endif

#endif
