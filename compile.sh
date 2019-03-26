#!/bin/bash
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

