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
PROJECT(${PLUGIN_PROCESS_MONITOR_PROJECT_NAME})

#Set include
SET(PLUGIN_PROCESS_MONITOR_INC
    ${PLUGIN_COMMON_DIR}
    ${PLUGIN_PROCESS_MONITOR_DIR}
    ${PLUGIN_PROCESS_MONITOR_DIR}/${PLATFORM}
    )

#Set headers
SET(PLUGIN_PROCESS_MONITOR_HEADERS
    ${PLUGIN_PROCESS_MONITOR_DIR}/process_monitor.h
    ${PLUGIN_PROCESS_MONITOR_DIR}/process_monitor_types.h
    ${PLUGIN_PROCESS_MONITOR_DIR}/system_process.h
    ${PLUGIN_PROCESS_MONITOR_DIR}/${PLATFORM}/system_process_impl.h
    )

#Set sources
SET(PLUGIN_PROCESS_MONITOR_SOURCES
    ${PLUGIN_PROCESS_MONITOR_DIR}/process_monitor.cpp
    ${PLUGIN_PROCESS_MONITOR_DIR}/${PLATFORM}/system_process_impl.cpp
    ${REFERENCE_APP_DIR}/data_network.cpp
    )

#Include Directories
INCLUDE_DIRECTORIES ( ${PLUGIN_PROCESS_MONITOR_INC}
                      ${PLUGIN_PMU_PUBLISHER_INC}
                      ${API_DIR}
                      ${EXTERNAL_TBB_DIR}/include
                    )

#Windows Specific Operations
IF (${PLATFORM} STREQUAL "windows")
    # Configure Windows RC file
    createPluginRcFile(${PLUGIN_PROCESS_MONITOR_DIR})

    SET( PLUGIN_PROCESS_MONITOR_SOURCES ${PLUGIN_PROCESS_MONITOR_SOURCES}
         ${PLUGIN_PROCESS_MONITOR_DIR}/${PROJECT_NAME}.rc)

    #Windows Code Signing Process
    configurePluginSigningCommand()
ENDIF (${PLATFORM} STREQUAL "windows")

#Create shared library
ADD_LIBRARY(${PROJECT_NAME} SHARED ${PLUGIN_PROCESS_MONITOR_HEADERS} ${PLUGIN_PROCESS_MONITOR_SOURCES})

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
