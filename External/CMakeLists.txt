#********************************************************************************
#    Copyright (C) 2019 Intel Corporation
#    SPDX-License-Identifier: BSD-3-Clause
#********************************************************************************
#
#    @file   CMakeLists.txt
#
#    @brief   External libraries CMake file script
#
#
#********************************************************************************
CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

#Features that requires external git capabilities
IF (ENABLE_KAFKA OR ENABLE_HOTSPOT)
    FIND_PACKAGE(Git REQUIRED)
ENDIF(ENABLE_KAFKA OR ENABLE_HOTSPOT)


# function to download and unpack external libraries
# TODO: use CMAKE_PARSE_ARGUMENTS
FUNCTION(DownloadLibrary LibraryName LibraryDir FileURL FileHash GitURL GitBranch GitCommit)
    IF (GIT_FOUND)
        IF (NOT EXISTS ${LibraryDir})
            IF (NOT ${FileURL} STREQUAL "")

                GET_FILENAME_COMPONENT(DownloadFileName ${FileURL} NAME)

                IF (NOT EXISTS ${EXTERNAL_DIR}/download/${DownloadFileName})
                    MESSAGE("Downloading ${LibraryName}")
                    MESSAGE("    from ${FileURL}")
                    MESSAGE("    as ${EXTERNAL_DIR}/download/${DownloadFileName}")
                    FILE(DOWNLOAD ${FileURL} ${EXTERNAL_DIR}/download/${DownloadFileName}
                        SHOW_PROGRESS
                        EXPECTED_HASH SHA256=${FileHash} )
                ENDIF (NOT EXISTS ${EXTERNAL_DIR}/download/${DownloadFileName})

                MESSAGE("Extracting ${LibraryName}")
                MESSAGE("    from ${EXTERNAL_DIR}/download/${DownloadFileName}")
                MESSAGE("    to ${LibraryDir}")
                EXECUTE_PROCESS(
                    COMMAND ${CMAKE_COMMAND} -E tar xfz ${EXTERNAL_DIR}/download/${DownloadFileName}
                    WORKING_DIRECTORY ${EXTERNAL_DIR}
                    )

            ELSEIF (NOT ${GitURL} STREQUAL "")

                MESSAGE("Cloning ${LibraryName} from ${GitURL}")

                EXECUTE_PROCESS(
                    COMMAND ${GIT_EXECUTABLE} clone "${GitURL}" "${LibraryDir}"
                    WORKING_DIRECTORY ${EXTERNAL_DIR}
                    OUTPUT_VARIABLE DOWNLOAD_GIT_STATUS
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )

                IF (NOT ${GitBranch} STREQUAL "")
                    EXECUTE_PROCESS(
                        COMMAND ${GIT_EXECUTABLE} checkout "${GitBranch}"
                        WORKING_DIRECTORY ${LibraryDir}
                        )
                ENDIF (NOT ${GitBranch} STREQUAL "")

                IF (NOT ${GitCommit} STREQUAL "")
                    EXECUTE_PROCESS(
                        COMMAND ${GIT_EXECUTABLE} reset --hard "${GitCommit}"
                        WORKING_DIRECTORY ${LibraryDir}
                        )
                ENDIF (NOT ${GitCommit} STREQUAL "")
            ELSE ()
                MESSAGE(FATAL_ERROR "Unable to download ${LibraryName}")
            ENDIF (NOT ${FileURL} STREQUAL "")
        ELSE ()
            MESSAGE("Found ${LibraryName} at ${LibraryDir}, skipping download...")
        ENDIF (NOT EXISTS ${LibraryDir})
    ELSE ()
        MESSAGE( WARNING "Git is not installed! Unable to download " ${LibraryName} )
    ENDIF (GIT_FOUND)
ENDFUNCTION()

# Download Boost
IF (NOT EXISTS ${EXTERNAL_BOOST_DIR}/boost)
    MESSAGE ("Downloading Boost..")
    FILE(DOWNLOAD https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.zip ${EXTERNAL_BOOST_DIR}/download/boost_1_68_0.zip
        SHOW_PROGRESS
        EXPECTED_HASH SHA256=3b1db0b67079266c40b98329d85916e910bbadfc3db3e860c049056788d4d5cd )
    MESSAGE ("Unpacking Boost..")
    #Execute cmake command tar to uncompress the zip file. (cmake uses its own compressor implementation)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${EXTERNAL_BOOST_DIR}/download/boost_1_68_0.zip
        WORKING_DIRECTORY   ${EXTERNAL_DIR})
ENDIF (NOT EXISTS ${EXTERNAL_BOOST_DIR}/boost)

# Bootstrap Boost
IF (${PLATFORM} STREQUAL  "linux")
    IF (NOT EXISTS ${EXTERNAL_BOOST_DIR}/b2)
        MESSAGE ("Bootstraping Boost..")
        EXECUTE_PROCESS(COMMAND chmod +x ${EXTERNAL_BOOST_DIR}/bootstrap.sh
            WORKING_DIRECTORY   ${EXTERNAL_BOOST_DIR})
        EXECUTE_PROCESS(COMMAND chmod +x ${EXTERNAL_BOOST_DIR}/tools/build/src/engine/build.sh
            WORKING_DIRECTORY   ${EXTERNAL_BOOST_DIR})
        EXECUTE_PROCESS(COMMAND ${EXTERNAL_BOOST_DIR}/bootstrap.sh
            WORKING_DIRECTORY   ${EXTERNAL_BOOST_DIR})
        EXECUTE_PROCESS(COMMAND chmod +x ${EXTERNAL_BOOST_DIR}/b2
            WORKING_DIRECTORY   ${EXTERNAL_BOOST_DIR})
    ENDIF (NOT EXISTS ${EXTERNAL_BOOST_DIR}/b2)
ELSEIF (${PLATFORM} STREQUAL  "windows")
    # Don't run boost bootstrap with the Ninja generator
    # Bootstrapping boost_1_68_0 fails with Ninja fails (boost_1_67_0 was okay)
    # Run_build.py now runs the Visual Studio generator first so we don't want to do this anyway.
    IF ((NOT EXISTS ${EXTERNAL_BOOST_DIR}/b2.exe) AND (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja"))
        MESSAGE ("Bootstraping Boost..")
        EXECUTE_PROCESS(COMMAND ${EXTERNAL_BOOST_DIR}/bootstrap.bat vc14
            WORKING_DIRECTORY   ${EXTERNAL_BOOST_DIR})
    ENDIF ((NOT EXISTS ${EXTERNAL_BOOST_DIR}/b2.exe) AND (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja"))
ENDIF (${PLATFORM} STREQUAL  "linux")

# Build Boost (skip for Ninja generator)
IF ((NOT EXISTS ${EXTERNAL_BOOST_DIR}/stage) AND (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja"))
    MESSAGE ("Building Boost..")
    IF (${PLATFORM} STREQUAL  "linux")
        SET(COMPILER_FLAGS cflags=-fPIC)
    ENDIF (${PLATFORM} STREQUAL  "linux")
    SET (NUMBER_OF_PARALLEL_THREADS 1)
    IF (DEFINED ENV{CMAKE_BUILD_PARALLEL_LEVEL})
        SET(NUMBER_OF_PARALLEL_THREADS $ENV{CMAKE_BUILD_PARALLEL_LEVEL})
    ENDIF(DEFINED ENV{CMAKE_BUILD_PARALLEL_LEVEL})
    EXECUTE_PROCESS(COMMAND ${EXTERNAL_BOOST_DIR}/b2 threading=multi runtime-link=static ${COMPILER_FLAGS} --abbreviate-paths -j${NUMBER_OF_PARALLEL_THREADS} --with-system --with-filesystem --with-program_options --with-atomic --with-date_time
        WORKING_DIRECTORY   ${EXTERNAL_BOOST_DIR})
ENDIF ((NOT EXISTS ${EXTERNAL_BOOST_DIR}/stage) AND (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja"))

# Download binaries for Intel TBB
IF (NOT EXISTS ${EXTERNAL_TBB_DIR}/include)
    IF (${PLATFORM} STREQUAL  "linux")
        MESSAGE ("Downloading Intel Thread Building Blocks..")
        FILE(DOWNLOAD https://github.com/01org/tbb/releases/download/2019/tbb2019_20180718oss_lin.tgz ${EXTERNAL_TBB_DIR}/download/tbb2019_20180718oss_lin.tgz
            SHOW_PROGRESS
            EXPECTED_HASH SHA256=7CA91043834EA662D93083FFD0D9BADCF9580FD596BC8C352386A7AE49EB6336 )
        MESSAGE ("Unpacking Intel Thread Building Blocks..")
        #Execute cmake command tar to uncompress the zip file. (cmake uses its own compressor implementation)
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${EXTERNAL_TBB_DIR}/download/tbb2019_20180718oss_lin.tgz
            WORKING_DIRECTORY   ${EXTERNAL_DIR})
    ELSEIF (${PLATFORM} STREQUAL  "windows")
        MESSAGE ("Downloading Intel Thread Building Blocks..")
        FILE(DOWNLOAD https://github.com/01org/tbb/releases/download/2019/tbb2019_20180718oss_win.zip ${EXTERNAL_TBB_DIR}/download/tbb2019_20180718oss_win.zip
            SHOW_PROGRESS
            EXPECTED_HASH SHA256=497BEAA673D59AF24D641CB586D9C54C44ECD52D0793685513909EB6B74B71AC )
        MESSAGE ("Unpacking Intel Thread Building Blocks..")
        #Execute cmake command tar to uncompress the zip file. (cmake uses its own compressor implementation)
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${EXTERNAL_TBB_DIR}/download/tbb2019_20180718oss_win.zip
            WORKING_DIRECTORY   ${EXTERNAL_DIR})
    ENDIF (${PLATFORM} STREQUAL  "linux")
ENDIF (NOT EXISTS ${EXTERNAL_TBB_DIR}/include)

# Download XED
IF (ENABLE_HOTSPOT)
    DownloadLibrary("XED" ${EXTERNAL_XED_DIR} "" "" "https://github.com/intelxed/xed.git" "" "7d62c8c49b7bb48de5512196610ad1689b3e5cee")

    # Download mbuild
    DownloadLibrary("mbuild" ${EXTERNAL_MBUILD_DIR} "" "" "https://github.com/intelxed/mbuild.git" "" "1651029643b2adf139a8d283db51b42c3c884513")

    IF (NOT EXISTS ${EXTERNAL_XED_OBJ_DIR})
        MESSAGE ("Building XED...")
        IF (${PLATFORM} STREQUAL  "linux")
            EXECUTE_PROCESS(COMMAND ./mfile.py --silent --extra-ccflags=-fPIC WORKING_DIRECTORY ${EXTERNAL_XED_DIR})
        ELSEIF (${PLATFORM} STREQUAL  "windows")
            EXECUTE_PROCESS(COMMAND python mfile.py --silent --no-encoder WORKING_DIRECTORY ${EXTERNAL_XED_DIR})
        ENDIF (${PLATFORM} STREQUAL  "linux")
    ENDIF (NOT EXISTS ${EXTERNAL_XED_OBJ_DIR})
ENDIF (ENABLE_HOTSPOT)

#Download and compile openssl
IF (ENABLE_KAFKA AND (NOT EXISTS ${OPENSSL_DIR}/download) AND (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja"))
    SET (OPEN_SSL_VERSION openssl-1.0.2q)
    SET (OPEN_SSL_FILE  ${OPEN_SSL_VERSION}.tar.gz)

    getVisualStudioEnvironmentScript()
    MESSAGE ("Downloading openssl..")
    FILE(DOWNLOAD https://www.openssl.org/source/${OPEN_SSL_FILE} ${OPENSSL_DIR}/download/${OPEN_SSL_FILE}
        SHOW_PROGRESS
        EXPECTED_HASH SHA256=5744cfcbcec2b1b48629f7354203bc1e5e9b5466998bbccc5b5fcde3b18eb684 )
    MESSAGE ("Unpacking openssl..")
    #Execute cmake command tar to uncompress the zip file. (cmake uses its own compressor implementation)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${OPENSSL_DIR}/download/${OPEN_SSL_FILE}
        WORKING_DIRECTORY   ${OPENSSL_DIR})

    MESSAGE ("Building openssl for Windows..")

    EXECUTE_PROCESS(COMMAND cmd /c call ${VISUAL_STUDIO_ENVIRONMENT} x64 && perl Configure VC-WIN64A --prefix=${OPENSSL_DIR}/install
                            && call "ms\\do_win64a.bat" && nmake -f ms/ntdll.mak && nmake -f ms/ntdll.mak install
        WORKING_DIRECTORY   ${OPENSSL_DIR}/${OPEN_SSL_VERSION})

ENDIF (ENABLE_KAFKA AND (NOT EXISTS ${OPENSSL_DIR}/download) AND (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja"))

IF (ENABLE_KAFKA AND(NOT EXISTS ${KAFKA_CLIENT_DIR}/download) AND (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja"))
    SET (LIBRDKAFKA_FILE    v${LIBRDKAFKA_VERSION}.zip)
    MESSAGE ("Downloading librdkafka ..")
    FILE(DOWNLOAD https://github.com/edenhill/librdkafka/archive/${LIBRDKAFKA_FILE} ${KAFKA_CLIENT_DIR}/download/${LIBRDKAFKA_FILE}
        SHOW_PROGRESS
        EXPECTED_HASH SHA256=ffbb76cd60e47fe4d6512ecdfb59ab6411308a78c7c7a5764efbaae5119c40de )
    MESSAGE ("Unpacking librdkafka..")
    #Execute cmake command tar to uncompress the zip file. (cmake uses its own compressor implementation)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${KAFKA_CLIENT_DIR}/download/${LIBRDKAFKA_FILE}
        WORKING_DIRECTORY   ${KAFKA_CLIENT_DIR})

    MESSAGE ("Building librdkafka..")
    EXECUTE_PROCESS(COMMAND cmd /c SET OPENSSL_ROOT_DIR=${OPENSSL_DIR}/install & mkdir build & cd build
                            & ${CMAKE_COMMAND} -DCMAKE_CXX_FLAGS_RELEASE:STRING="/MT" -DCMAKE_CXX_FLAGS_DEBUG:STRING="/MTd" .. -DRDKAFKA_BUILD_TESTS=OFF -DRDKAFKA_BUILD_EXAMPLES=OFF -G ${CMAKE_GENERATOR}
                            & ${CMAKE_COMMAND} --build .  --clean-first --config Release
                            & ${CMAKE_COMMAND} --build .  --clean-first --config Debug
        WORKING_DIRECTORY   ${KAFKA_CLIENT_DIR}/librdkafka-${LIBRDKAFKA_VERSION})
ENDIF (ENABLE_KAFKA AND(NOT EXISTS ${KAFKA_CLIENT_DIR}/download) AND (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja"))