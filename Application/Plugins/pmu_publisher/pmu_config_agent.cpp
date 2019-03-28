/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   pmu_config_agent.h
**
**    @brief  Helper class for creating the PMU configuration for heuristics
**
**
********************************************************************************
*/

#include <boost/algorithm/string/predicate.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "pmu_config_agent.h"

namespace bit_shovel_plugins
{
    namespace internal
    {
        /**
         * @brief Event Select Register format
         * According to
         * "Intel 64 and IA-32 Architectures Software Developers Manual Volume 3B:
         * System Programming Guide, Part 2", Figure 30-6. Layout of IA32_PERFEVTSELx
         * MSRs Supporting Architectural Performance Monitoring Version 3
         */
        struct EventSelectRegister
        {
            union
            {
                struct
                {
                    uint64_t eventSelect : 8;
                    uint64_t umask : 8;
                    uint64_t usr : 1;
                    uint64_t os : 1;
                    uint64_t edge : 1;
                    uint64_t pinControl : 1;
                    uint64_t apicInt : 1;
                    uint64_t anyThread : 1;
                    uint64_t enable : 1;
                    uint64_t invert : 1;
                    uint64_t cmask : 8;
                    uint64_t inTx : 1;
                    uint64_t inTxcp : 1;
                    uint64_t reservedX : 30;
                } fields;
                uint64_t value;
            };
        };

        // placeholder value for a string that isn't a fixed event
        constexpr uint32_t const_invalid_fixed_evenum = 255u;

        // total number of fixed events available
        constexpr uint32_t const_num_hw_fixed_events = 3u;

        class pmu_config_agent_impl
        {
        public:
            using tokenizer_t = boost::tokenizer<boost::char_separator<char>>;

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
            static bool try_get_value(tokenizer_t::iterator& it, T& out)
            {
                // backup iterator
                auto my_it = it;

                if ((++my_it).at_end() || *my_it != "=" || (++my_it).at_end())
                {
                    return false;
                }

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

            static bit_shovel::result_type parse_event_string(const std::string& event_string,
                hw_telemetry_configuration_t& out,
                hw_pmu_events_slots_t& pmu_events_slots)
            {
                bit_shovel::result_type result;  // success by default

                // setup struct to gather register config
                internal::EventSelectRegister reg;
                reg.value = 0;
                reg.fields.usr = 1;
                reg.fields.os = 0;
                reg.fields.enable = 1;

                // fixed data
                uint32_t fixed_eventnum = const_invalid_fixed_evenum;

                // interrupt data
                uint32_t interrupt_rate = 0;

                // offcore data
                uint64_t offcore_value = 0;

                // setup boost tokenizer
                boost::char_separator<char> separators(
                    /* separators to drop =*/",/\\", /* separators to keep =*/"=");
                tokenizer_t tokenizer(event_string, separators);

                bool is_event = false;
                bool is_fixed = false;

                for (tokenizer_t::iterator it = tokenizer.begin(); it != tokenizer.end(); ++it)
                {
                    uint64_t u64_temp;
                    std::string str_temp;

                    if (*it == "cpu")
                    {
                        // nothing to do here
                    }
                    else if (*it == "event")
                    {
                        if (try_get_value<uint64_t>(it, u64_temp))
                        {
                            // sadly can't pass a bitfield by reference, so assign it here
                            reg.fields.eventSelect = u64_temp;
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'event' must be numeric." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "umask")
                    {
                        if (try_get_value<uint64_t>(it, u64_temp))
                        {
                            // sadly can't pass a bitfield by reference, so assign it here
                            reg.fields.umask = u64_temp;
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'umask' must be numeric." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "edge")
                    {
                        // valid must be 0 or 1 (or a bare entry, i.e. no equals sign)
                        auto my_it = it;
                        if ((++my_it).at_end() || *my_it != "=")
                        {
                            reg.fields.edge = 1;
                            is_event = true;
                            ++it;
                        }
                        else
                        {
                            bool got_value = try_get_value<uint64_t>(it, u64_temp);
                            if (got_value && (u64_temp == 0 || u64_temp == 1))
                            {
                                reg.fields.edge = u64_temp;
                                is_event = true;
                            }
                            else
                            {
                                result.add_failure()
                                    << "Event string option 'edge' must be either 0 or 1."
                                    << std::endl
                                    << "  " << event_string << std::endl;
                            }
                        }
                    }
                    else if (*it == "fixed")
                    {
                        // valid range is 0 - 2
                        bool got_value = try_get_value<uint32_t>(it, fixed_eventnum);
                        if (got_value && fixed_eventnum < const_num_hw_fixed_events)
                        {
                            is_fixed = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'fixed' must be either 0, 1, or 2."
                                << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "int")
                    {
                        if (try_get_value<uint32_t>(it, interrupt_rate))
                        {
                            // turn on interrupt flag
                            reg.fields.apicInt = 1;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'int' must be numeric." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "any")
                    {
                        // valid must be 0 or 1
                        bool got_value = try_get_value<uint64_t>(it, u64_temp);
                        if (got_value && (u64_temp == 0 || u64_temp == 1))
                        {
                            reg.fields.anyThread = u64_temp;
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'any' must be either 0 or 1." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "inv")
                    {
                        // valid must be 0 or 1
                        bool got_value = try_get_value<uint64_t>(it, u64_temp);
                        if (got_value && (u64_temp == 0 || u64_temp == 1))
                        {
                            reg.fields.invert = u64_temp;
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'inv' must be either 0 or 1." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "cmask")
                    {
                        if (try_get_value<uint64_t>(it, u64_temp))
                        {
                            // sadly can't pass a bitfield by reference, so assign it here
                            reg.fields.cmask = u64_temp;
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'cmask' must be numeric." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "in_tx")
                    {
                        // valid must be 0 or 1
                        bool got_value = try_get_value<uint64_t>(it, u64_temp);
                        if (got_value && (u64_temp == 0 || u64_temp == 1))
                        {
                            reg.fields.inTx = u64_temp;
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'in_tx' must be either 0 or 1." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "in_tx_cp")
                    {
                        // valid must be 0 or 1
                        bool got_value = try_get_value<uint64_t>(it, u64_temp);
                        if (got_value && (u64_temp == 0 || u64_temp == 1))
                        {
                            reg.fields.inTxcp = u64_temp;
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'in_tx_cp' must be either 0 or 1."
                                << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "pc")
                    {
                        // valid must be 0 or 1
                        bool got_value = try_get_value<uint64_t>(it, u64_temp);
                        if (got_value && (u64_temp == 0 || u64_temp == 1))
                        {
                            reg.fields.pinControl = u64_temp;
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'pc' must be either 0 or 1." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "offcore_rsp")
                    {
                        if (try_get_value<uint64_t>(it, offcore_value))
                        {
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'offcore_rsp' must be numeric." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else if (*it == "name")
                    {
                        if (try_get_value<std::string>(it, str_temp))
                        {
                            is_event = true;
                        }
                        else
                        {
                            result.add_failure()
                                << "Event string option 'name' is missing." << std::endl
                                << "  " << event_string << std::endl;
                        }
                    }
                    else
                    {
                        // invalid string options
                        result.add_failure()
                            << "Event string option '" << *it << "' found in config." << std::endl
                            << "  " << event_string << std::endl;
                    }

                    // cannot use both sets of options at the same time!
                    if (result && is_event && is_fixed)
                    {
                        // invalid string options
                        result.add_failure() << "Event string found with both fixed and "
                                                "programmable event options set:"
                                             << std::endl
                                             << "  " << event_string << std::endl;
                    }

                    if (!result) break;
                }

                // update the main config!
                if (result)
                {
                    if (is_event)
                    {
                        size_t i = 0;

                        // find unused event
                        for (; i < sizeof(out.eventSel) / sizeof(out.eventSel[0]); i++)
                        {
                            if (out.eventSel[i] == 0) break;
                        }

                        if (i < sizeof(out.eventSel) / sizeof(out.eventSel[0]))
                        {
                            // event selection is mandatory
                            if (reg.fields.eventSelect == 0)
                            {
                                result.add_failure()
                                    << "Found programmable event string with no event select:"
                                    << std::endl
                                    << "  " << event_string << std::endl;
                            }

                            if (result && offcore_value != 0)
                            {
                                // offcore only valid for event_0 and event_1
                                if (i > 1)
                                {
                                    result.add_failure()
                                        << "Offcore events are only valid for counters 0 and 1 "
                                           "(first two programmable event type event strings):"
                                        << std::endl
                                        << "  " << event_string << std::endl;
                                }

                                out.offCore[i] = offcore_value;
                            }

                            if (result)
                            {
                                if (interrupt_rate != 0)
                                {
                                    out.samplesPerInterrupt[i] = interrupt_rate;
                                }

                                out.eventSel[i] = reg.value;
                                pmu_events_slots[i] = event_string;
                            }
                        }
                        else
                        {
                            // TOO MANY EVENTS
                            result.add_failure() << "More than four programmable events found in "
                                                    "event strings in config."
                                                 << std::endl;
                        }
                    }
                    else if (is_fixed)
                    {
                        if (interrupt_rate != 0 && fixed_eventnum != const_invalid_fixed_evenum)
                        {
                            out.samplesPerInterruptFixed[fixed_eventnum] = interrupt_rate;
                        }
                        else
                        {
                            // must have valid interrupt rate and fixed event number
                            result.add_failure() << "Must specify interrupt threshold for fixed "
                                                    "counters in event strings config:"
                                                 << std::endl
                                                 << "  " << event_string << std::endl;
                        }
                    }
                    else
                    {
                        // need to have one or the other!
                        result.add_failure() << "Found event string that is neither a programmable "
                                                "event nor a fixed counter:"
                                             << std::endl
                                             << "  " << event_string << std::endl;
                    }
                }

                return result;
            }
        };
    }  // namespace internal

    bit_shovel::result_type pmu_config_agent::parse(const hw_pmu_events_config_t& config,
        hw_telemetry_configuration_t& out,
        hw_pmu_events_slots_t& pmu_events_slots)
    {
        bit_shovel::result_type result;  // success by default

        if (!config.empty())
        {

            for (auto& item : config)
            {
                // update config with event string
                result = internal::pmu_config_agent_impl::parse_event_string(
                    item, out, pmu_events_slots);
                if (!result) break;
            }
        }
        else
        {
            result.add_failure() << "No event strings found in configuration. Unable to setup PMU."
                                 << std::endl;
        }

        return result;
    }

}  // namespace bit_shovel_plugins
