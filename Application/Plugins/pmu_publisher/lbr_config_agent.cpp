/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   lbr_config_agent.cpp
**
**    @brief  Helper class for creating the LBR configuration for heuristics
**
**
********************************************************************************
*/

#include <sstream>
#include <cctype>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "lbr_config_agent.h"

namespace bit_shovel_plugins
{
    using tokenizer_t = boost::tokenizer<boost::char_separator<char>>;

    namespace internal
    {
        class lbr_config_agent_impl
        {
        public:
            // helper class to add hex string support to boost::lexical_cast
            template<class T>
            class T_from_hex
            {
                T value;

            public:
                operator T() const
                {
                    return value;
                }
                friend std::istream& operator>>(std::istream& in, T_from_hex& out)
                {
                    in >> std::hex >> out.value;
                    return in;
                }
            };

            template<class T>
            static bool try_get_value_if_key_equals(tokenizer_t::iterator& it,
                const std::string key,
                T& out)
            {
                // backup iterator
                auto my_it = it;

                if (*my_it == key && !(++my_it).at_end() && *my_it == "=" && !(++my_it).at_end())
                {
                    try
                    {
                        if (boost::starts_with(*my_it, "0x"))
                        {
                            out = boost::lexical_cast<T_from_hex<T>>(*my_it);
                        }
                        else
                        {
                            out = boost::lexical_cast<T>(*my_it);
                        }
                    }
                    catch (const boost::bad_lexical_cast&)
                    {
                        // bad string!
                        return false;
                    }

                    // update iterator if we were successful
                    it = my_it;

                    return true;
                }

                return false;
            }

            /** @brief Sets one of the lbr_select bits in the driver/HW telemetry configuration.
             *
             * @param cfg (OUT) The driver/HW telemetry configuration
             * @param bitNum The number of the bit to set in lbr_select
             * @param cfgVal A value of 1 to enable the bit or 0 to disable the bit
             * @param result (OUT) The result object.  An error will be added if cfgVal is invalid.
             */
            static void set_lbr_select_bit(hw_telemetry_configuration_t& cfg,
                size_t bitNum,
                short cfgVal,
                bit_shovel::result_type& result)
            {
                if (cfgVal == 1)
                    cfg.lbr_select |= (1ULL << bitNum);
                else if (cfgVal == 0)
                    cfg.lbr_select &= ~(1ULL << bitNum);
                else
                    result.add_failure() << "Invalid config value for lbr_select bit " << bitNum
                                         << " (not 1 or 0)" << std::endl;
            }

            static bit_shovel::result_type parse_lbr_config_string(const std::string& config_string,
                hw_telemetry_configuration_t& out,
                hw_lbr_config_opts_t& lbr_config_opts)
            {
                bit_shovel::result_type result;  // success by default

                // U64 out.U64 lbr_select;     // value to be programmed into MSR_LBR_SELECT
                // Documentation for the settings for LBRs can be seen here:
                // http://sorami-chi.hateblo.jp/entry/2017/12/17/230000
                // The settings are in table 17-11: MSR_LBR_SELECT for Nahalem architecture
                // Bit Field      Bit Offset   Access  Description
                // CPL_EQ_0       0            R/W     When set, do not capture branches occuring in
                // ring 0 CPL_NEQ_0      1            R/W     When set, do not capture branches
                // occuring in ring >0 JCC            2            R/W     When set, do not capture
                // conditional branches NEAR_REL_CALL  3            R/W     When set, do not capture
                // near relative calls NEAR_IND_CALL  4            R/W     When set, do not capture
                // near indirect calls NEAR_RET       5            R/W     When set, do not capture
                // near returns NEAR_IND_JMP   6            R/W     When set, do not capture near
                // indirect jumps NEAR_REL_JMP   7            R/W     When set, do not capture near
                // relative jumps FAR_BRANCH     8            R/W     When set, do not capture far
                // branches Reserved       63:9                 Must be zero
                const std::string lbr_opt_names[] = {"CPL_EQ_0",
                    "CPL_NEQ_0",
                    "JCC",
                    "NEAR_REL_CALL",
                    "NEAR_IND_CALL",
                    "NEAR_RET",
                    "NEAR_IND_JUMP",
                    "NEAR_REL_JUMP",
                    "FAR_BRANCH"};
                size_t bit_num = 0;
                boost::char_separator<char> separators(
                    /* separators to drop =*/",/\\", /* separators to keep =*/"=");
                // Convert the config string to uppercase so we can do case-insensitive
                // matching/parsing.
                std::string config_string_upper = boost::algorithm::to_upper_copy(config_string);
                tokenizer_t tokenizer(config_string_upper, separators);
                for (tokenizer_t::iterator it = tokenizer.begin(); it != tokenizer.end(); ++it)
                {
                    short short_temp = 0;
                    for (const std::string& lbr_opt_name : lbr_opt_names)
                    {
                        if (try_get_value_if_key_equals<short>(it, lbr_opt_name, short_temp))
                        {
                            set_lbr_select_bit(out, bit_num++, short_temp, result);
                            // If the option was enabled, add its name to lbr_config_opts.
                            if (short_temp == 1) lbr_config_opts->push_back(lbr_opt_name);
                        }
                    }
                }

                return result;
            }
        };
    }  // namespace internal

    bit_shovel::result_type lbr_config_agent::parse(const lbr_config_t& config,
        hw_telemetry_configuration_t& out,
        hw_lbr_config_opts_t& lbr_config_opts)
    {
        out.lbr_enable =
            (config.lbr_enable == true ? TRUE
                                       : FALSE);  // Converting C++ bool to #defined TRUE/FALSE
        out.num_lbr_entries_requested =
            (decltype(out.num_lbr_entries_requested))config.num_lbrs_to_collect;
        // Parse the LBR configuration string into the MSR_LBR_SELECT bits
        bit_shovel::result_type result = internal::lbr_config_agent_impl::parse_lbr_config_string(
            config.msr_lbr_select_config, out, lbr_config_opts);
        return result;
    }

}  // namespace bit_shovel_plugins
