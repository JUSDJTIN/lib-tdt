#********************************************************************************
#    Intel Architecture Group
#    Copyright (C) 2019 Intel Corporation
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