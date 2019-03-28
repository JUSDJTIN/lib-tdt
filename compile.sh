#!/bin/bash
#********************************************************************************
#    Copyright (C) 2019 Intel Corporation
#    SPDX-License-Identifier: BSD-3-Clause
#********************************************************************************
#
#    @file   compile.sh
#
#    @brief  Helper shell script to compile TDT library using devtoolset-7
#
#
#********************************************************************************
source /opt/rh/devtoolset-7/enable
if [ $? -ne 0 ];
then
    echo "Error enabling devtoolset-7"
    exit 1
fi

if [ ! -f "CMakeLists.txt" ];
then
    echo "Error: Current directory does not contain CMakeLists.txt"
    exit 2
fi

mkdir build
if [ $? -ne 0 ];
then
    echo "Error creating build directory"
    exit 3
fi

cd build
if [ $? -ne 0 ];
then
    echo "Error changing to build directory"
    exit 4
fi

CMAKE_BUILD_PARALLEL_LEVEL=$(nproc)
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ];
then
    echo "Error executing cmake to create make files"
    exit 5
fi

cmake --build . --clean-first --config Release
if [ $? -ne 0 ];
then
    echo "Error compiling the solution"
    exit 5
fi

echo "Succesfully compiled"
exit 0

