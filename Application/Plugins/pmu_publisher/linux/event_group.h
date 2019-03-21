/*
 * ********************************************************************************
 *     Intel Architecture Group
 *     Copyright (C) 2018 Intel Corporation
 * ********************************************************************************
 *
 *     INTEL CONFIDENTIAL
 *     This file, software, or program is supplied under the terms of a
 *     license agreement and/or nondisclosure agreement with Intel Corporation
 *     and may not be copied or disclosed except in accordance with the
 *     terms of that agreement.  This file, software, or program contains
 *     copyrighted material and/or trade secret information of Intel
 *     Corporation, and must be treated as such.  Intel reserves all rights
 *     in this material, except as the license agreement or nondisclosure
 *     agreement specifically indicate.
 *
 *     All rights reserved.  No part of this program or publication
 *     may be reproduced, transmitted, transcribed, stored in a
 *     retrieval system, or translated into any language or computer
 *     language, in any form or by any means, electronic, mechanical,
 *     magnetic, optical, chemical, manual, or otherwise, without
 *     the prior written permission of Intel Corporation.
 *
 *     Intel makes no warranty of any kind regarding this code.  This code
 *     is provided on an "As Is" basis and Intel will not provide any support,
 *     assistance, installation, training or other services.  Intel does not
 *     provide any updates, enhancements or extensions.  Intel specifically
 *     disclaims any warranty of merchantability, noninfringement, fitness
 *     for any particular purpose, or any other warranty.
 *
 *     Intel disclaims all liability, including liability for infringement
 *     of any proprietary rights, relating to use of the code.  No license,
 *     express or implied, by estoppel or otherwise, to any intellectual
 *     property rights is granted herein.
 */
/********************************************************************************
 **
 **    @file   event_group.h
 **
 **    @brief  perf_event group management
 **
 **
 ********************************************************************************
 */

#ifndef EVENT_GROUP_H
#define EVENT_GROUP_H

#include <memory>
#include <vector>
#include <linux/perf_event.h>

#include "pmu_event.h"
#include "pmu_publisher_types.h"

namespace bit_shovel_plugins
{

    /**
     * Helper class for managing a group of pmu perf events.
     */
    class event_group
    {
    public:
        /**
         * @brief Construct a event_group
         *
         */
        event_group();

        /**
         * @brief destruct the event group
         * disable and close all the event file descriptors
         */
        virtual ~event_group();

        event_group(const event_group&) = default;
        event_group& operator=(const event_group&) = default;

        /**
         * @brief Append the list of events to the group, first event is the group leader
         *
         * @param event
         */
        void add(std::vector<pmu_event>& events);

        /**
         * @brief Opens the events in the group
         *
         * @return
         */
        bool open();

        /**
         * @brief Enable the event group
         *
         * @return true
         */
        bool enable();

        /**
         * @brief Disable the event group
         *
         * @return true
         */
        bool disable() const;

        /**
         * @brief Get this groups cpu
         *
         * @return the cpu number
         */
        uint cpu() const
        {
            return m_cpu;
        }

        /**
         * @brief Set this groups cpu
         *
         * @param cpu the cpu this event group is for
         */
        void cpu(const uint16_t cpu)
        {
            m_cpu = cpu;
        }

        /**
         * @brief Get the file descriptor for the group
         *
         * @return
         */
        int file_descriptor() const
        {
            return m_leader_fd;
        }

        /**
         * @brief Get the telemetry records from the CPU mmap ring buffer.
         *
         * The ring buffer contains perf sample records which are converted to
         * PMI telemetry records.
         *
         * @param records
         * @return
         */
        size_t get_telemetry_records(hw_telemetry_records_t& records);

    private:
        /**
         * Structure of the counter samples in the perf mmap ring buffer samples.
         * Derived from comments in perf_event.h
         */
        struct read_format_t
        {
            uint64_t nr;

            struct values_t
            {
                uint64_t value;
                uint64_t id;
            } values[];  // size depends on number of events configured
            // NOTE: refer to <linux/perf_event.h> when additional bits are added to read_format.
        };

        /**
         * Structure of the perf mmap ring buffer samples.  Derived from comments in perf_event.h
         */
        struct sample_data_record_t
        {
            //    pe.sample_type = PERF_SAMPLE_TID | PERF_SAMPLE_TIME | PERF_SAMPLE_READ |
            //    PERF_SAMPLE_CPU;
            perf_event_header header;
            uint32_t pid;  /* if PERF_SAMPLE_TID */
            uint32_t tid;  /* if PERF_SAMPLE_TID */
            uint64_t time; /* if PERF_SAMPLE_TIME */
            uint32_t cpu;
            uint32_t res;    /* if PERF_SAMPLE_CPU */
            read_format_t v; /* if PERF_SAMPLE_READ */
            // NOTE: refer to <linux/perf_event.h> when additional bits are added to sample_type.
        };

        /**
         * @brief Create the mmap ring buffer for data collection
         *
         * @param data_pages number of pages for data in the mmap ring buffer
         *
         * @return true if the mmap ring buffer was successfully created.
         */
        bool _create_mmap_buffer(const uint8_t data_pages = NUM_DATA_PAGES);

        /**
         * @brief Copy the perf sample bytes from the mmap buffer to m_data;
         *
         * @return the number of bytes in the internal data buffer 'm_data'
         */
        size_t _get_data();

        /**
         * @brief determine how much data is available and the position
         *
         * @param[out] pos position of the bytes in the mmap ring buffer
         * @return number of bytes available in the mmap ring buffer
         */
        const size_t _data_available(size_t& pos);

        /**
         * @brief Update the ring buffer metadata tail
         *
         * @param length the number of bytes extracted from the ring buffer
         */
        void _update_mmap_tail(const size_t length);

        /**
         * @brief  Convert the perf sample record to a telemetry record
         *
         * @param sample_record
         * @param telemetry_record
         */
        void _convert_to_telemetry_record(const sample_data_record_t* sample_record,
            tdtsdk_pmi_record_t& telemetry_record);
        /**
         * @brief Close all events in the group
         */
        void _close();

        /**
         * The number pages used for data in the mmap ring buffer.
         * TODO: this may need to be tuned
         */
        static constexpr uint8_t NUM_DATA_PAGES = 2;

        /**
         * The CPU associated with this group of events
         */
        uint16_t m_cpu;

        /**
         * The events in the group
         */
        std::vector<pmu_event> m_events;

        /**
         * The file descriptor of the group leader event.
         */
        int m_leader_fd;

        /**
         * The bytes retrieved from the mmap ring buffer for conversion to PMI Records for the
         * pipeline.
         *
         * TODO: Investigate converting directly from the mmap ring buffer.
         */
        std::shared_ptr<std::vector<uint8_t> > m_data;

        /**
         * The previous values of the counters to make them relative like the TDT Windows Driver
         * where the counters are reset on each interrupt.
         */
        uint64_t m_previous[MAX_APP_FIXED_EVENTS + MAX_APP_PROG_EVENTS] = {0};

        //----------------------------------------------------------------------
        // mmap ring buffer variables

        /**
         * pointer to the perf mmap buffer meta data
         */
        struct perf_event_mmap_page* m_mmap_metadata = nullptr;
        /**
         * pointer to the perf sample data in the mmap memory buffer
         */
        uint8_t* m_mmap_data = nullptr;

        /**
         * Totoal size of the mmap ring buffer
         */
        size_t m_mmap_buffer_size = 0;

        /**
         * Size of the mmap ring buffer containing the sample data
         */
        size_t m_data_buffer_size = 0;

        /**
         * Pointer to the mmap ring buffer (return value from mmap call)
         */
        void* m_mmap_ring_buffer = nullptr;
    };

}  // namespace bit_shovel_plugins
#endif /* EVENT_GROUP_H */
