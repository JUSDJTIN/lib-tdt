/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   config_util.h
**
**    @brief  Helper functions to manage config profile merges
**
**
********************************************************************************
*/
#ifndef CONFIG_PROFILE_UTIL_H
#define CONFIG_PROFILE_UTIL_H

#include <string>
#include <vector>
#include <map>

#include <boost/property_tree/ptree.hpp>

#include "plugin_api_types.h"

namespace config_util
{

    // forward declaration
    struct config_path_sorter;

    /**
     * @brief Data structure to hold all config elements as path, value string pairs.
     * @note The value may contain more than one element for empty child key leaf nodes
     */
    using flattened_config_t = std::map<std::string, std::vector<std::string>, config_path_sorter>;

    /**
     * @brief Helper structure to sort the flattened configuration map with longer paths (e.g. leaf
     * nodes) on top of the list
     * @note This is desirable because overwriting a node in a ptree erases all previous children.
     *       By adding the leaf nodes first we implicit create all the parents which can
     *       merely have their values updated later as needed.
     */
    struct config_path_sorter
    {

        /**
         * @brief Sort longer strings first, then alphabetically
         * @param lhs left hand string
         * @param rhs right hand string
         * @return returns true if lhs should be before rhs when sorting
         */
        bool operator()(const std::string& lhs, const std::string& rhs) const
        {
            // longer strings go first!
            if (lhs.size() > rhs.size())
            {
                return true;
            }
            else if (lhs.size() < rhs.size())
            {
                return false;
            }

            // equal length means alphabetical sorting
            // this is required since the ordering must be deterministic if lhs and rhs are switched
            // also, two identical string should return false. (see C++ strict weak ordering
            // requirements)
            return lhs < rhs;
        }
    };

    // helper functions
    //----------------------------------------------------------------------------

    /**
     * @brief Check if a config path leads to a anonymous leaf node (e.g. with empty string as the
     * name of the property)
     * @note Build-in ptree path methods sadly do not handle anonymous property nodes.
     *       Also, this code does not handle non-leaf anonymous property nodes.
     * @param path Child path to check
     * @return return true if that path points to an anonymous leaf node
     */
    inline static bool is_path_to_anonymous_node(const std::string& path)
    {
        return path.empty() ? false : path[path.size() - 1] == '.';
    }

    /**
     * @brief Recursive helper function to change a boost ptree into a flatten pair of <full path,
     * property value>
     * @param config The node (including children) which we want to add to the flattened config
     * structure
     * @param flattened_out The "flattened" data structure to populate with new entries
     * @param cur_path The path from the absolute config root to the element passed in as 'config'
     */
    inline static void flatten_config_helper(const bit_shovel::plugin_config_t& config,
        flattened_config_t& flattened_out,
        std::string cur_path = "")
    {
        // process all children of this node first (DFS)
        for (const auto& child_info : config)
        {
            flatten_config_helper(child_info.second,
                flattened_out,
                (cur_path.empty() ? "" : cur_path + ".") + child_info.first);
        }

        // next, accumulate if empty child key
        if (is_path_to_anonymous_node(cur_path))
        {
            flattened_out[cur_path].push_back(config.data());
        }
        // otherwise, store / replace
        else
        {
            flattened_out[cur_path] = {config.data()};
        }
    }

    /**
     * @brief Function to convert ptree and all children into a flattened config structure
     * @param config The ptree config to convert
     * @return The resulting flattened data structure
     */
    inline static flattened_config_t flatten_config(const bit_shovel::plugin_config_t& config)
    {
        flattened_config_t flat_config;
        flatten_config_helper(config, flat_config);
        return flat_config;
    }

    /**
     * @brief Copy, with overwrite on conflict, from one flattened config to another
     * @param dest The flattened config structure to receive possible new entries or overwrites
     * @param source The flattened config structure to copy nodes from
     */
    inline static void overwrite_merge_flattened_config(flattened_config_t& dest,
        const flattened_config_t& source)
    {
        // for each node in source
        for (const auto& kvp : source)
        {
            // accumulate nodes for empty keys
            if (is_path_to_anonymous_node(kvp.first))
            {
                for (const auto& item : kvp.second)
                {
                    dest[kvp.first].push_back(item);
                }
            }
            // otherwise just replace
            else
            {
                dest[kvp.first] = kvp.second;
            }
        }
    }

    /**
     * @brief Function to convert flattened config structure back to a ptree
     * @param flat_config The flattened config structure to convert
     * @param tree_config The ptree to populate with the new values
     */
    inline static void unflatten_config(const flattened_config_t& flat_config,
        bit_shovel::plugin_config_t& tree_config)
    {
        for (const auto& kvp : flat_config)
        {
            // handle root case
            if (!kvp.first.empty())
            {
                // handle anonymous key case
                if (is_path_to_anonymous_node(kvp.first))
                {
                    const auto parent_path = kvp.first.substr(0, kvp.first.size() - 1);
                    auto parent_opt = tree_config.get_child_optional(parent_path);

                    if (!parent_opt.is_initialized())
                    {
                        parent_opt = tree_config.put(parent_path, "");
                    }

                    for (const auto& value : kvp.second)
                    {
                        parent_opt.value().push_back(
                            std::make_pair("", bit_shovel::plugin_config_t(value)));
                    }
                }
                else
                {
                    auto child_opt = tree_config.get_child_optional(kvp.first);
                    if (!child_opt.is_initialized())
                    {
                        tree_config.put(kvp.first, kvp.second[0]);
                    }
                    else
                    {
                        child_opt.value().put_value(kvp.second[0]);
                    }
                }
            }
            else
            {
                tree_config.put_value(kvp.second[0]);
            }
        }
    }

}  // namespace config_util

#endif  // CONFIG_PROFILE_UTIL_H