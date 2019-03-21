#********************************************************************************
#    Intel Architecture Group
#    Copyright (C) 2018 Intel Corporation
#********************************************************************************
#
#    INTEL CONFIDENTIAL
#    This file, software, or program is supplied under the terms of a
#    license agreement and/or nondisclosure agreement with Intel Corporation
#    and may not be copied or disclosed except in accordance with the
#    terms of that agreement.  This file, software, or program contains
#    copyrighted material and/or trade secret information of Intel
#    Corporation, and must be treated as such.  Intel reserves all rights
#    in this material, except as the license agreement or nondisclosure
#    agreement specifically indicate.
#
#    All rights reserved.  No part of this program or publication
#    may be reproduced, transmitted, transcribed, stored in a
#    retrieval system, or translated into any language or computer
#    language, in any form or by any means, electronic, mechanical,
#    magnetic, optical, chemical, manual, or otherwise, without
#    the prior written permission of Intel Corporation.
#
#    Intel makes no warranty of any kind regarding this code.  This code
#    is provided on an "As Is" basis and Intel will not provide any support,
#    assistance, installation, training or other services.  Intel does not
#    provide any updates, enhancements or extensions.  Intel specifically
#    disclaims any warranty of merchantability, noninfringement, fitness
#    for any particular purpose, or any other warranty.
#
#    Intel disclaims all liability, including liability for infringement
#    of any proprietary rights, relating to use of the code.  No license,
#    express or implied, by estoppel or otherwise, to any intellectual
#    property rights is granted herein.
#
#********************************************************************************
#
#    @file   CMakeLists.txt
#
#    @brief   Top level CMake file script
#
#
#********************************************************************************
SET(CMAKE_MINIMUM_REQUIRED_VERSION  3.11.2)
CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

#Set PLATFORM variable (linux, windows)
STRING(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} PLATFORM)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

#Include functions and directories
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeFunctions.txt)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeTargetFunctions.txt)

#Configure compiler flags
configureCompilerFlags()

configureVersion()

#Solution name
PROJECT("ThreatDetectionTechnology")
ENABLE_LANGUAGE(CXX)

SET(${PROJECT_NAME}_VERSION ${TDT_VERSION_MAJOR}.${TDT_VERSION_MINOR}.${TDT_VERSION_RELEASE}.${TDT_VERSION_BUILD})

#Project names
SET(TDT_APPLICATION_PROJECT_NAME                    TDTReferenceApplication)
SET(TDT_LIBRARY_TEST_APPLICATION_PROJECT_NAME       TDTLibraryTestApp)
SET(TDT_LIBRARY_PROJECT_NAME                        tdt)
SET(TEST_PROJECT_NAME                               Tests)
SET(TEST_DRIVER_PROJECT_NAME                        runDriverTests)
SET(TEST_TDT_LIBRARY_PROJECT_NAME                   test_library_api)
SET(TEST_TDT_LIB_C_API_PROJECT_NAME                 test_lib_c_api)
SET(WINDOWS_SIGNING_LIBRARY_PROJECT_NAME            SignVerifyLib)
SET(NORMALIZER_LIB                                  StaticNormalizerLib)
SET(PLUGIN_BOISE_HEURISTIC_PROJECT_NAME             boise_heuristic)
SET(PLUGIN_CONSOLE_REPORTER_PROJECT_NAME            console_reporter)
SET(PLUGIN_LIBRARY_REPORTER_PROJECT_NAME            library_reporter)
SET(PLUGIN_CSV_TELEMETRY_RECORDER_PROJECT_NAME      csv_telemetry_recorder)
SET(PLUGIN_FILE_REPORTER_PROJECT_NAME               file_reporter)
SET(PLUGIN_PMU_PUBLISHER_PROJECT_NAME               pmu_publisher)
SET(PLUGIN_NORMALIZER_PROJECT_NAME                  normalizer)
SET(PLUGIN_PROCESS_MONITOR_PROJECT_NAME             process_monitor)
SET(PLUGIN_RANDOM_FOREST_CLASSIFIER_PROJECT_NAME    random_forest_classifier)
SET(PLUGIN_TELEMETRY_DATA_REPLAYER_PROJECT_NAME     telemetry_data_replayer)
SET(PLUGIN_HOTSPOT_DETECTOR_PROJECT_NAME            hotspot_detector)
SET(PLUGIN_KAFKA_REPORTER_PROJECT_NAME              kafka_reporter)

#Set build types for windows: Debug/Release
configureBuildTypes()

#Set output directories
configureOutputDirectories()

#Configure deliverable package
configureDeliverablePackage()

#include directories
ADD_SUBDIRECTORY(${EXTERNAL_DIR})
GetExternalLibrariesReferences()
ADD_SUBDIRECTORY(${TDT_LIBRARY_DIR})

addAdditionalProjects()
