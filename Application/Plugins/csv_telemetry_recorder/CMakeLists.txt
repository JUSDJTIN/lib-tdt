#********************************************************************************
#    Copyright (C) 2019 Intel Corporation
#    SPDX-License-Identifier: BSD-3-Clause
#********************************************************************************
#
#    @file   CMakeLists.txt
#
#    @brief  Plugin CMake file script
#
#
#********************************************************************************
CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

#Project name
PROJECT(${PLUGIN_CSV_TELEMETRY_RECORDER_PROJECT_NAME})

#Set include
SET(PLUGIN_CSV_TELEMETRY_RECORDER_INC
    ${PLUGIN_CSV_TELEMETRY_RECORDER_DIR}
    )

#Set headers
SET(PLUGIN_CSV_TELEMETRY_RECORDER_HEADERS
    ${PLUGIN_CSV_TELEMETRY_RECORDER_DIR}/csv_telemetry_recorder.h
    ${PLUGIN_CSV_TELEMETRY_RECORDER_DIR}/csv_telemetry_recorder_types.h
    ${PLUGIN_CSV_TELEMETRY_RECORDER_DIR}/csv_telemetry_recorder.inl
    )

#Set sources
SET(PLUGIN_CSV_TELEMETRY_RECORDER_SOURCES
    ${PLUGIN_CSV_TELEMETRY_RECORDER_DIR}/csv_telemetry_recorder.cpp
    ${REFERENCE_APP_DIR}/data_network.cpp
    )

#Include Directories
INCLUDE_DIRECTORIES ( ${PLUGIN_CSV_TELEMETRY_RECORDER_DIR}
                      ${PLUGIN_PMU_PUBLISHER_INC}
                    )

#Windows Specific Operations
IF (${PLATFORM} STREQUAL "windows")
    # Configure Windows RC file
    createPluginRcFile(${PLUGIN_CSV_TELEMETRY_RECORDER_DIR})

    SET( PLUGIN_CSV_TELEMETRY_RECORDER_SOURCES ${PLUGIN_CSV_TELEMETRY_RECORDER_SOURCES}
         ${PLUGIN_CSV_TELEMETRY_RECORDER_DIR}/${PROJECT_NAME}.rc)

    #Windows Code Signing Process
    configurePluginSigningCommand()
ENDIF (${PLATFORM} STREQUAL "windows")

#Create shared library
ADD_LIBRARY(${PROJECT_NAME} SHARED ${PLUGIN_CSV_TELEMETRY_RECORDER_HEADERS} ${PLUGIN_CSV_TELEMETRY_RECORDER_SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                      ${VALIDATION_LIBRARIES}
                      ${Boost_LIBRARIES}
                      ${TBB_IMPORTED_TARGETS}
                      )

ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${PLUGIN_SIGNING_COMMAND}
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${RELEASE_PACKAGE_DIR}
                   COMMAND ${CMAKE_COMMAND} -E copy ${LIB_DEST_DIR}${PROJECT_NAME}${LIB_EXT} ${RELEASE_PACKAGE_DIR}/.
                   )
