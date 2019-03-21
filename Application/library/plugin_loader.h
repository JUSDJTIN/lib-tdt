/*
 ** *******************************************************************************
 **     Intel Architecture Group
 **     Copyright (C) 2019 Intel Corporation.
 ** ********************************************************************************
 **
 **     INTEL CONFIDENTIAL
 **     This file, software, or program is supplied under the terms of a
 **     license agreement and/or nondisclosure agreement with Intel Corporation
 **     and may not be copied or disclosed except in accordance with the
 **     terms of that agreement.  This file, software, or program contains
 **     copyrighted material and/or trade secret information of Intel
 **     Corporation, and must be treated as such.  Intel reserves all rights
 **     in this material, except as the license agreement or nondisclosure
 **     agreement specifically indicate.
 **
 **     All rights reserved.  No part of this program or publication
 **     may be reproduced, transmitted, transcribed, stored in a
 **     retrieval system, or translated into any language or computer
 **     language, in any form or by any means, electronic, mechanical,
 **     magnetic, optical, chemical, manual, or otherwise, without
 **     the prior written permission of Intel Corporation.
 **
 **     Intel makes no warranty of any kind regarding this code.  This code
 **     is provided on an "As Is" basis and Intel will not provide any support,
 **     assistance, installation, training or other services.  Intel does not
 **     provide any updates, enhancements or extensions.  Intel specifically
 **     disclaims any warranty of merchantability, noninfringement, fitness
 **     for any particular purpose, or any other warranty.
 **
 **     Intel disclaims all liability, including liability for infringement
 **     of any proprietary rights, relating to use of the code.  No license,
 **     express or implied, by estoppel or otherwise, to any intellectual
 **     property rights is granted herein.
 ** /
 */
/********************************************************************************
 **
 **      @file   plugin_loader.h
 **
 **      @brief  Plugin loader interface description
 **
 **
 ********************************************************************************/

#ifndef PLUGINS_H
#define PLUGINS_H

namespace tdt_library
{
    /**
     * @brief Load the requested plugin
     *
     * @param[in] plugin_load_info String describing the plugin name optionally with a custom config
     * profile list
     * @param config Profile to use with this plugin if it doesn't have a custom config profile list
     * @param plugins A container to which the newly loaded plugin should be added
     * @param print_target A reference to the output stream for logging
     * @return Status code for error handling
     */
    tdt_return_code load_plugin(const bit_shovel::plugin_load_info_t& plugin_load_info,
        std::unique_ptr<bit_shovel::plugin_config_t> config,
        bit_shovel::plugin_list_t& plugins,
        std::ostream& print_target);

    /**
     * @brief Unloads all currently loaded plugins.
     *
     */
    void unload_all_plugins();
}  // namespace tdt_library

#endif /* PLUGINS_H */
