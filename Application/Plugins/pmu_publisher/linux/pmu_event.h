/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   pmu_event.h
**
**    @brief  perf_event helper definition
**
**
********************************************************************************
*/

#ifndef PMU_EVENT_H
#define PMU_EVENT_H

#ifdef DEBUG
#    include <iostream>
#endif

#include <memory>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <linux/perf_event.h>

namespace bit_shovel_plugins
{
    namespace internal
    {
        /**
         * Wakeup every 10 events
         * This may need to be tuned.
         */
        constexpr auto WAKE_EVENT_COUNT = 10;

        /**
         * Return value of -1 returned by read/open/etc.
         */
        constexpr int IO_ERROR = -1;

    }  // namespace internal

    /**
     * @brief Helper class for perf events.
     *
     * This class is a wrapper for the 'perf_event_attr' structure and provides
     * the necessary operation for using 'perf' events.
     * The TDT implementation uses PERF_TYPE_RAW to measure events for all
     * processes per CPU.
     */
    class pmu_event
    {
    public:
        /**
         * @brief Construct a perf event
         * Initialize the perf event attribute structure to default values.
         */
        pmu_event(const perf_type_id type);

        virtual ~pmu_event();

        pmu_event(const pmu_event&) = default;
        pmu_event& operator=(const pmu_event&) = default;

        /**
         * @brief Specifies which event samples will be recorded for, with the type field
         * (PERF_TYPE_RAW).
         *
         * The type is PERF_TYPE_RAW, so value is a custom "raw" config value.
         * The driver masks the 'config' value with X86_RAW_EVENT_MASK (0x0084FFFF)
         * then adds the interrupt bit.
         *
         * @param config_value the "raw" custom config value.
         */
        void configure(const uint64_t config_value);

        /**
         * @brief Sets the sampling period for the event
         *
         * A "sampling" counter is one that generates an interrupt every N events, where N is
         * given by sample_period.
         * Note: The group leader must have a sampling period, if not not data is recorded.
         *
         * @param[in] period number of events to interrupt on
         */
        void sample_period(const uint64_t period);

        /**
         *  @brief Open the event.
         *
         * Make the system call to open the event.  See the perf_event_open man page for details.
         * All events in the group are opened with a call to perv_event_open().
         *
         * @param[in] pid  The PID this event is to measure (-1 for all processes)
         * @param[in] cpu  The CPU this event is to measure
         * @param[in] group_fd  The File descriptor of the group leader (-1 for the group leader)
         * @param[in] flags flags for the event (not used at this time)
         * @return false if the perf_event_open system call fails
         */
        bool open(const pid_t pid,
            const int32_t cpu,
            const int32_t group_fd,
            const uint64_t flags = 0);

        /**
         * @brief Enable the event (start event recording)
         * When the group leader is enabled the kernel module enables all the member of the group
         * Note: all counters are reset before enabling.
         *
         * @return false if ioctl() fails.
         */
        bool enable() const;

        /**
         * @brief Disable the event (stop event recording)
         * All events in a group must be disabled explicitly.
         */
        void disable() const;

        /**
         * @brief Close the event
         * All events in a group must be closed explicitly.
         */
        void close();

        /**
         * @brief get the file descriptor
         * Used by event_group class to get the file descriptor of the group leader.
         *
         * @return the file descriptor, IO_ERROR if the event has not been opened.
         */
        int file_descriptor() const
        {
            return m_fd;
        }

        /**
         * @brief get the event ID
         * @return the event ID
         */
        uint64_t id() const
        {
            return m_id;
        }

        /**
         * @brief Check for event group leadership
         *
         * @return true if the event is the group leader
         */
        bool is_leader() const
        {
            return m_leader;
        }

        /**
         * @brief Make this event a group leader
         */
        void leader()
        {
            m_leader = true;
        }

        /**
         * @brief Get the event type, used for determining Fixed or Programmable event.
         *
         * @return the Event type one of the types in perf_type_id enumeration.
         */
        uint32_t type() const
        {
            return m_attr.type;
        }

    public:
        /**
         * Indicates that this event is not group leader in open()
         */
        static constexpr int32_t NO_GROUP = -1;
        /**
         * Indicates that the Events measure all processes in open()
         */
        static constexpr pid_t ALL_PROCESSES = -1;
        /**
         * Indicates that the Events measure the current process in open()
         */
        static constexpr pid_t CURRENT_PROCESS = 0;

    private:
#ifdef DEBUG_VERBOSE

        void _debug_print(const std::string& from)
        {
            std::cout << from << "()\t"
                      << " ID : " << m_id << " FD : " << m_fd << (is_leader() ? " LEADER" : "")
                      << " Type: " << m_attr.type << " size: " << m_attr.size << " config: 0x"
                      << std::hex << m_attr.config << std::dec << " disabled: " << m_attr.disabled
                      << " exclude K: " << m_attr.exclude_kernel
                      << " exclude HV: " << m_attr.exclude_hv << " period: " << m_attr.sample_period
                      << " freq: " << m_attr.sample_freq << std::endl;
        }
#else

        void _debug_print(const std::string& from){};
#endif

    private:
        /**
         * @brief The event file descriptor
         */
        int m_fd;
        /**
         * the event ID value for the given event file descriptor from PERF_EVENT_IOC_ID
         */
        uint64_t m_id;
        /**
         * @brief flag to indicate this event is the group leader
         */
        bool m_leader;
        /**
         * @brief The event attributes
         */
        perf_event_attr m_attr;
    };

}  // namespace bit_shovel_plugins
#endif /* PMU_EVENT_H */
