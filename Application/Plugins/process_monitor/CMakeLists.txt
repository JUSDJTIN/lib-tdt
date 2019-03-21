#********************************************************************************
#    Intel Architecture Group
#    Copyright (C) 2010-2018 Intel Corporation
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
