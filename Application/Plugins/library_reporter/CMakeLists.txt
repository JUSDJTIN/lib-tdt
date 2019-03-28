#********************************************************************************
#    Copyright (C) 2019 Intel Corporation
#    SPDX-License-Identifier: BSD-3-Clause
#********************************************************************************
#
#    @file   CMakeLists.txt
#
#    @brief  Plugin library_reporter CMake file script
#
#
#********************************************************************************
CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

#Project name
PROJECT(${PLUGIN_LIBRARY_REPORTER_PROJECT_NAME})

#Set include
SET(PLUGIN_LIBRARY_REPORTER_INC
    ${PLUGIN_LIBRARY_REPORTER_DIR}
    ${PLUGIN_COMMON_DIR}
    )

#Set headers
SET(PLUGIN_LIBRARY_REPORTER_HEADERS
    ${PLUGIN_LIBRARY_REPORTER_DIR}/library_reporter.h
    ${PLUGIN_LIBRARY_REPORTER_DIR}/library_reporter.inl
    )

#Set sources
SET(PLUGIN_LIBRARY_REPORTER_SOURCES
    ${PLUGIN_LIBRARY_REPORTER_DIR}/library_reporter.cpp
    ${REFERENCE_APP_DIR}/data_network.cpp
    )

#Include Directories
INCLUDE_DIRECTORIES ( ${PLUGIN_LIBRARY_REPORTER_DIR}
                      )

#Windows Specific Operations
IF (${PLATFORM} STREQUAL "windows")
    # Configure Windows RC file
    createPluginRcFile(${PLUGIN_LIBRARY_REPORTER_DIR})

    SET( PLUGIN_LIBRARY_REPORTER_SOURCES ${PLUGIN_LIBRARY_REPORTER_SOURCES}
        ${PLUGIN_LIBRARY_REPORTER_DIR}/${PROJECT_NAME}.rc)

    #Windows Code Signing Process
    configurePluginSigningCommand()
ENDIF (${PLATFORM} STREQUAL "windows")

#Create shared library
ADD_LIBRARY(${PROJECT_NAME} SHARED ${PLUGIN_LIBRARY_REPORTER_HEADERS} ${PLUGIN_LIBRARY_REPORTER_SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                      ${Boost_LIBRARIES}
                      ${TBB_IMPORTED_TARGETS}
                      )

ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${PLUGIN_SIGNING_COMMAND}
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${RELEASE_PACKAGE_DIR}
                   COMMAND ${CMAKE_COMMAND} -E copy ${LIB_DEST_DIR}${PROJECT_NAME}${LIB_EXT} ${RELEASE_PACKAGE_DIR}/.
                  )