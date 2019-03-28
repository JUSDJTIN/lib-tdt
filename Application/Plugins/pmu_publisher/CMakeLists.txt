#********************************************************************************
#    Copyright (C) 2019 Intel Corporation
#    SPDX-License-Identifier: BSD-3-Clause
#********************************************************************************
#
#    @file   CMakeLists.txt
#
#    @brief  Plugin CMake file script for the PMU Publisher
#
#
#********************************************************************************
CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

#Project name
PROJECT(${PLUGIN_PMU_PUBLISHER_PROJECT_NAME})

# OS specific files
IF (${PLATFORM} STREQUAL  "linux")
    SET( PMU_PUBLISHER_OS_INC ${DRIVER_INCLUDE_DIR} )
    SET( PMU_PUBLISHER_OS_HEADERS
         ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/pmu_event.h
         ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/event_group.h
        )
    SET( PMU_PUBLISHER_OS_SOURCES
        ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/pmu_event.cpp
        ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/event_group.cpp
        )
ELSEIF (${PLATFORM} STREQUAL  "windows")
    SET( PMU_PUBLISHER_OS_INC ${DRIVER_WINDOWS_INC_DIR})
    SET( PMU_PUBLISHER_OS_HEADERS
        ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/shared_memory_comm.h
        )
    SET( PMU_PUBLISHER_OS_SOURCES
        ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/shared_memory_comm.cpp
        )
ENDIF  (${PLATFORM} STREQUAL  "linux")

#Set include
SET(PLUGIN_PMU_PUBLISHER_INC
    ${PLUGIN_PMU_PUBLISHER_DIR}
    ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}
    ${DRIVER_INCLUDE_HEADERS}
    ${PMU_PUBLISHER_OS_INC}
    )

#Set headers
SET(PLUGIN_PMU_PUBLISHER_HEADERS
    ${PLUGIN_PMU_PUBLISHER_DIR}/pmu_config_agent.h
    ${PLUGIN_PMU_PUBLISHER_DIR}/pmu_publisher.h
    ${PLUGIN_PMU_PUBLISHER_DIR}/pmu_publisher_types.h
    ${PLUGIN_PMU_PUBLISHER_DIR}/lbr_config_agent.h
    ${PLUGIN_PMU_PUBLISHER_DIR}/lbr_config.h
    ${PLUGIN_PMU_PUBLISHER_DIR}/driver_interface.h
    ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/driver_interface_impl.h
    ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/driver.h
    ${PMU_PUBLISHER_OS_HEADERS}
    )

#Set sources
SET(PLUGIN_PMU_PUBLISHER_SOURCES
    ${PLUGIN_PMU_PUBLISHER_DIR}/pmu_config_agent.cpp
    ${PLUGIN_PMU_PUBLISHER_DIR}/pmu_publisher.cpp
    ${PLUGIN_PMU_PUBLISHER_DIR}/lbr_config_agent.cpp
    ${PLUGIN_PMU_PUBLISHER_DIR}/${PLATFORM}/driver_interface_impl.cpp
    ${REFERENCE_APP_DIR}/data_network.cpp
    ${PMU_PUBLISHER_OS_SOURCES}
    )

#Include Directories
INCLUDE_DIRECTORIES (${PLUGIN_PMU_PUBLISHER_DIR}
                     ${Boost_INCLUDE_DIRS}
                     ${TDT_LIBRARY_DIR})

#Windows Specific Operations
IF (${PLATFORM} STREQUAL "windows")
    # Configure Windows RC file
    createPluginRcFile(${PLUGIN_PMU_PUBLISHER_DIR})

    SET( PLUGIN_PMU_PUBLISHER_SOURCES ${PLUGIN_PMU_PUBLISHER_SOURCES}
         ${PLUGIN_PMU_PUBLISHER_DIR}/${PROJECT_NAME}.rc)

    #Windows Code Signing Process
    configurePluginSigningCommand()
ENDIF (${PLATFORM} STREQUAL "windows")

#Create shared library
ADD_LIBRARY(${PROJECT_NAME} SHARED ${PLUGIN_PMU_PUBLISHER_HEADERS} ${PLUGIN_PMU_PUBLISHER_SOURCES})

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
