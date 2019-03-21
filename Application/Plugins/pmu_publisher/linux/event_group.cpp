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
 **    @file   event_group.cpp
 **
 **    @brief  perf_event group management implementation
 **
 **
 ********************************************************************************
 */

#ifdef DEBUG
#    include <iostream>
#endif
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <errno.h>
#include "event_group.h"

namespace bit_shovel_plugins
{

    event_group::event_group()
        : m_cpu(UINT16_MAX)
        , m_events()
        , m_leader_fd(internal::IO_ERROR)
        , m_data(nullptr)
        , m_previous{0}
    {}

    event_group::~event_group()
    {
        if (!m_events.empty())
        {
            // clean up the events (disable and close)
            m_events[0].disable();
            _close();
            m_events.clear();
        }
        // Release the mmap ring buffer
        if (m_mmap_ring_buffer != nullptr)
        {
            munmap(m_mmap_ring_buffer, m_mmap_buffer_size);
            m_mmap_ring_buffer = nullptr;
        }
    }

    void event_group::add(std::vector<pmu_event>& events)
    {
        events[0].leader();
        m_events = events;
    }

    bool event_group::open()
    {
        bool success = false;
        for (auto& event : m_events)
        {
            // The first call will have value of NO_GROUP (-1)
            m_leader_fd = m_events[0].file_descriptor();
            success = event.open(pmu_event::ALL_PROCESSES, m_cpu, m_leader_fd);
            if (!success)
            {
                _close();
                break;
            }
        }

        // When all the opens are successful create the MMap ring buffer
        if (success)
        {

            success = _create_mmap_buffer();
            if (!success)
            {
                _close();
            }
        }
        return success;
    }

    bool event_group::enable()
    {
        bool success = false;
        for (const auto& event : m_events)
        {
            if (event.is_leader())
            {
                success = event.enable();
                break;
            }
        }
        return success;
    }

    bool event_group::disable() const
    {
        bool success = false;
        for (const auto& event : m_events)
        {
            if (event.is_leader())
            {
                event.disable();
                break;
            }
        }
        return success;
    }

    void event_group::_convert_to_telemetry_record(const sample_data_record_t* sample,
        tdtsdk_pmi_record_t& telemetry_record)
    {

        if (sample != nullptr)
        {
#ifdef DEBUG_SAMPLE
            std::cout << "Sample : "
                      << " type = PERF_RECORD_SAMPLE(" << sample->header.type << ")"
                      << " misc = " << sample->header.misc << " size = " << sample->header.size
                      << " pid = " << sample->pid << " tid = " << sample->tid
                      << " CPU = " << sample->cpu << " time = " << sample->time
                      << " NR = " << sample->v.nr << std::endl;
#endif

            telemetry_record.cpu = sample->cpu;
            telemetry_record.pid = sample->pid;
            telemetry_record.tid = sample->tid;

            // TODO: remove the conversion to 100 nanosecond units!
            telemetry_record.timeStamp = sample->time / 100;

            size_t fixed_index = 0;
            size_t prog_index = 0;
            for (auto i = 0u; i < sample->v.nr; ++i)
            {
#ifdef DEBUG_SAMPLE
                std::cout << __func__ << "() value[" << i << "] = " << sample->v.values[i].value
                          << std::endl;
#endif

                uint64_t delta = sample->v.values[i].value;
                if (sample->v.values[i].value >= m_previous[i])
                {
                    delta -= m_previous[i];
                }
                else
                {
                    delta += UINT64_MAX - m_previous[i];
                }
                m_previous[i] = sample->v.values[i].value;

                switch (m_events[i].type())
                {
                    case PERF_TYPE_RAW:
                        if (prog_index < MAX_APP_PROG_EVENTS)
                        {
                            telemetry_record.events_data[prog_index++] =
                                static_cast<uint32_t>(delta & UINT32_MAX);
                        }
                        break;

                    case PERF_TYPE_HARDWARE:
                        if (fixed_index < MAX_APP_FIXED_EVENTS)
                        {
                            telemetry_record.fixed_data[fixed_index++] =
                                static_cast<uint32_t>(delta & UINT32_MAX);
#if !defined(USE_CPU_CYCLES)
                            // Skip the CPU cycles since the
                            // HW_CPU_CYCLES event was not programmed.
                            ++fixed_index;
#endif
                        }
                        break;

                    default:
#ifdef DEBUG
                        std::cerr << "Invalid/Unsupported event type (" << m_events[i].type() << ")"
                                  << std::endl;
#endif
                        break;
                }
            }
        }
    }

    size_t event_group::get_telemetry_records(hw_telemetry_records_t& records)
    {
        const size_t SAMPLE_RECORD_SIZE =
            sizeof(sample_data_record_t) + ((sizeof(uint64_t) * 2) * m_events.size());
        std::vector<tdtsdk_pmi_record_t> telemetry;

        // Get the bytes from the mmap buffer
        const size_t len = _get_data();

        // Make sure there is at least 1 sample record
        if (len > 0)
        {
            auto rec_ptr = m_data->data();
            const auto end_ptr = rec_ptr + len;
            while (rec_ptr <= end_ptr)
            {
                const auto hdr = reinterpret_cast<perf_event_header*>(rec_ptr);
                if (hdr->size == 0)
                {
                    break;
                }

                switch (hdr->type)
                {
                    case PERF_RECORD_SAMPLE:
                        // Reserve space for the telemetry records.
                        if (telemetry.empty())
                        {
                            telemetry.reserve(len / SAMPLE_RECORD_SIZE);
                        }
                        tdtsdk_pmi_record_t telemetry_record;
                        _convert_to_telemetry_record(
                            reinterpret_cast<sample_data_record_t*>(rec_ptr), telemetry_record);
                        telemetry.emplace_back(telemetry_record);
                        break;

                    case PERF_RECORD_LOST:
#ifdef DEBUG
                        std::cerr << " LOST Records " << hdr->type << std::endl;
#endif
                        break;
                    case PERF_RECORD_THROTTLE:
#ifdef DEBUG
                        std::cerr << " PERF_RECORD_THROTTLE Record " << hdr->type << std::endl;
#endif
                        break;
                    case PERF_RECORD_UNTHROTTLE:
#ifdef DEBUG
                        std::cerr << " PERF_RECORD_UNTHROTTLE Record " << hdr->type << std::endl;
#endif
                        break;
                    default:
#ifdef DEBUG
                        std::cout << " WARNING Unsupported data type." << hdr->type << std::endl;
#endif
                        break;
                };
                rec_ptr += hdr->size;
            }
            if (telemetry.size() > 0)
            {
                records = std::make_shared<std::vector<tdtsdk_pmi_record_t>>(telemetry.size());
                std::copy(telemetry.begin(), telemetry.end(), records->begin());
            }
        }

        return telemetry.size();
    }

    bool event_group::_create_mmap_buffer(const uint8_t data_pages)
    {
        bool success = false;
        // TODO: Verify pages is a power of 2

        // Map pages (metadata data page + # pages)
        const size_t page_size = sysconf(_SC_PAGE_SIZE);
        m_mmap_buffer_size = (data_pages + 1) * page_size;

        // create mmap
        m_mmap_ring_buffer =
            mmap(nullptr, m_mmap_buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_leader_fd, 0);
        if (m_mmap_ring_buffer != MAP_FAILED)
        {
            m_mmap_metadata = reinterpret_cast<struct perf_event_mmap_page*>(m_mmap_ring_buffer);
            m_mmap_data = reinterpret_cast<uint8_t*>(m_mmap_ring_buffer) + page_size;
            m_data_buffer_size = m_mmap_buffer_size - page_size;

            const int status = fcntl(m_leader_fd, F_SETFL, O_NONBLOCK);
#ifdef DEBUG
            if (status == internal::IO_ERROR)
            {
                std::cerr << "fcntl failed :" << strerror(errno) << std::endl;
            }
#endif
            success = status != internal::IO_ERROR;
        }
#ifdef DEBUG
        else
        {
            std::cerr << "mmap failed :" << strerror(errno) << std::endl;
        }
#endif

        if (success)
        {
            m_data = std::make_shared<std::vector<uint8_t>>(m_data_buffer_size);
        }

        return success;
    }

    size_t event_group::_get_data()
    {
        size_t pos = 0;
        const size_t len = _data_available(pos);
#ifdef DEBUG_VERBOSE
        std::cout << __func__ << "(): CPU Event Group: " << m_cpu << " has " << len
                  << " bytes available @ " << pos << std::endl;
#endif

        if (len > 0)
        {
            uint8_t* p = m_data->data();
            memset(p, 0, m_data_buffer_size);
            const size_t copy_len = std::min(len, m_data_buffer_size - pos);
            memcpy(p, m_mmap_data + pos, copy_len);
            if (copy_len < len)
            {
                memcpy(p + copy_len, m_mmap_data, len - copy_len);
            }

            _update_mmap_tail(len);
        }
        return len;
    }

    void event_group::_update_mmap_tail(const size_t len)
    {
        // mb() used to ensure finish reading data before writing data_tail.
        __sync_synchronize();
        m_mmap_metadata->data_tail += len;
    }

    const size_t event_group::_data_available(size_t& pos)
    {
        const uint64_t write_head = m_mmap_metadata->data_head;
        const uint64_t read_head = m_mmap_metadata->data_tail;
        if (write_head != read_head)
        {
            // rmb() used to ensure reading data after reading data_head.
            __sync_synchronize();
            pos = read_head & (m_data_buffer_size - 1);
        }

        return write_head - read_head;
    }

    void event_group::_close()
    {
        for (auto& event : m_events)
        {
            event.close();
        }
    }
}  // namespace bit_shovel_plugins
