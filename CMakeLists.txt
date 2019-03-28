#********************************************************************************
#    Copyright (C) 2019 Intel Corporation
#    SPDX-License-Identifier: BSD-3-Clause
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
