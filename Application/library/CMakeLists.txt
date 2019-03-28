#********************************************************************************
#    Copyright (C) 2019 Intel Corporation
#    SPDX-License-Identifier: BSD-3-Clause
#********************************************************************************
#
#    @file   CMakeLists.txt
#
#    @brief  TDT Reference Application CMake file script
#
#
#********************************************************************************
CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

MESSAGE( STATUS "-- Creating TDT Interface Library Project...")

#Project name
PROJECT( ${TDT_LIBRARY_PROJECT_NAME}
    VERSION 99.0.0
    DESCRIPTION "TDT Library for ISecL"
    LANGUAGES CXX
    )

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

#Include Directories
INCLUDE_DIRECTORIES(
    ${PLUGIN_COMMON_DIR}
    ${PROJECT_SOURCE_DIR}/${PLATFORM}/
    ${PROJECT_SOURCE_DIR}
    ${API_DIR}
    ${Boost_INCLUDE_DIRS}
    ${EXTERNAL_TBB_DIR}/include
    ${SIGNVERIFY_LIB_DIR}
    )

#Source code files
SET( SOURCES
    ${PROJECT_SOURCE_DIR}/tdt_agent.cpp
    ${PROJECT_SOURCE_DIR}/tdt_agent_c_api_if.cpp
    ${PROJECT_SOURCE_DIR}/profile_loader.cpp
    ${PROJECT_SOURCE_DIR}/plugin_loader.cpp
    ${PROJECT_SOURCE_DIR}/${PLATFORM}/sign_handler_imp.cpp
    ${PROJECT_SOURCE_DIR}/pipeline_manager.cpp
    ${PROJECT_SOURCE_DIR}/data_network.cpp
    ${PROJECT_SOURCE_DIR}/channel_registry.cpp
    ${PROJECT_SOURCE_DIR}/json_utils.cpp
    )

#Header files
SET( HEADERS
    ${API_DIR}/data_source_base.h
    ${API_DIR}/simple_data_source.h
    ${API_DIR}/data_source_own_thread.h
    ${API_DIR}/data_network.h
    ${API_DIR}/internal/data_network.inl
    ${API_DIR}/channel_registry.h
    ${API_DIR}/internal/channel_registry.inl
    ${API_DIR}/plugin_api.h
    ${API_DIR}/result_type.h
    ${PROJECT_SOURCE_DIR}/profile_loader.h
    ${PROJECT_SOURCE_DIR}/plugin_loader.h
    ${PROJECT_SOURCE_DIR}/sign_handler_interface.h
    ${PROJECT_SOURCE_DIR}/${PLATFORM}/sign_handler_imp.h
    ${PROJECT_SOURCE_DIR}/pipeline_manager.h
    ${PROJECT_SOURCE_DIR}/config_util.h
    ${PROJECT_SOURCE_DIR}/lib_version.h
    ${PROJECT_SOURCE_DIR}/tdt_agent.hpp
    ${PROJECT_SOURCE_DIR}/tdt_agent.h
    ${PROJECT_SOURCE_DIR}/json_utils.h
    )

#Windows Specific Operations
IF (${PLATFORM} STREQUAL "windows")
    #The version info is not set when building from the source package.
    IF (TDT_VERSION_MAJOR)
        SET( TDT_STRFILE_DESCRIPTION "Intel (R) Threat Detection Technology Library." )
        CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/tdt_library.rc.in
            ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.rc)
    ENDIF (TDT_VERSION_MAJOR)

    SET( SOURCES ${SOURCES}
        ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.rc )

    IF (SIGNING_ENABLED)
        SET( LIB_SIGNING_COMMAND     "${SIGNFILE}" "${BUILD_DIR}/${CMAKE_CFG_INTDIR}/${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    ENDIF (SIGNING_ENABLED)

ENDIF (${PLATFORM} STREQUAL "windows")

# The source package includes the '*version.h' and the version
# variables are not set so do not regenerated the version header file
IF (TDT_VERSION_MAJOR)
    # Configure the version header file with the verison info.
    SET(${PROJECT_NAME}_VERSION_MAJOR ${TDT_VERSION_MAJOR})
    SET(${PROJECT_NAME}_VERSION_MINOR ${TDT_VERSION_MINOR})
    SET(${PROJECT_NAME}_VERSION_RELEASE ${TDT_VERSION_RELEASE})
    SET(${PROJECT_NAME}_VERSION_BUILD ${TDT_VERSION_BUILD})
    SET(${PROJECT_NAME}_VERSION_STR "${TDT_VERSION_MAJOR}.${TDT_VERSION_MINOR}.${TDT_VERSION_RELEASE}.${TDT_VERSION_BUILD}")

    CONFIGURE_FILE(
        ${PROJECT_SOURCE_DIR}/lib_version.h.in
        ${PROJECT_SOURCE_DIR}/lib_version.h
        )

ENDIF (TDT_VERSION_MAJOR)



IF (${PLATFORM} STREQUAL "linux")
    # Add the -shared flag so that the library is created correctly.
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shared")
    SET(LIB_PREFIX  lib)
ENDIF (${PLATFORM} STREQUAL "linux")


#Export API
ADD_DEFINITIONS(-DDLL_EXPORTS)

ADD_LIBRARY( ${PROJECT_NAME} SHARED
    ${SOURCES}
    ${HEADERS}
    )

SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES PUBLIC_HEADER tdt_agent.h
    PUBLIC_HEADER tdt_agent.hpp
    )

IF (${PLATFORM} STREQUAL "windows")
    #Windows dependencies
    TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${WINDOWS_SIGNING_LIBRARY_PROJECT_NAME})
ENDIF  (${PLATFORM} STREQUAL  "windows")


TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${TBB_IMPORTED_TARGETS}
    )

# Create named folders for the resources
SOURCE_GROUP("Resources" FILES ${RESOURCES_FILES})

#Post build event to to sign library and copy the library to release package
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${LIB_SIGNING_COMMAND}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RELEASE_PACKAGE_DIR}
    )

#Post build event to copy configuration to build directory
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CONFIGURATION_FILES_DIR}/${PLATFORM} ${BUILD_DIR}/${CMAKE_CFG_INTDIR}/.)

#Post build top copy tbb to build directory
IF (${PLATFORM} STREQUAL  "linux")
    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${EXTERNAL_TBB_LIBS}/$<$<CONFIG:debug>:libtbb_debug${CMAKE_SHARED_LIBRARY_SUFFIX}.2>$<$<CONFIG:release>:libtbb${CMAKE_SHARED_LIBRARY_SUFFIX}.2> ${BUILD_DIR}/${CMAKE_CFG_INTDIR}/.
        )
ELSEIF (${PLATFORM} STREQUAL  "windows")
    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${EXTERNAL_TBB_BINS}/$<$<CONFIG:debug>:tbb_debug.dll>$<$<CONFIG:release>:tbb.dll> ${BUILD_DIR}/${CMAKE_CFG_INTDIR}/.
        )
ENDIF  (${PLATFORM} STREQUAL  "linux")

#Post build event to copy precompiled binary files to release package
copyPrecompiledPluginToBinaryPackage()

#Post build event to copy build dir to release package
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${BUILD_DIR}/${CMAKE_CFG_INTDIR}/. ${RELEASE_PACKAGE_DIR}/.
    )

