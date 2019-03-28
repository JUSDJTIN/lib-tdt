/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   pmu_event.cpp
**
**    @brief  perf_event helper implementation
**
**
********************************************************************************
*/
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>

#include "pmu_event.h"

namespace bit_shovel_plugins
{

    pmu_event::pmu_event(const perf_type_id type)
        : m_fd(internal::IO_ERROR)
        , m_id(0)
        , m_leader(false)
        , m_attr{0}
    {
        m_attr.type = type;
        m_attr.size = sizeof(perf_event_attr);
        m_attr.disabled = 1;
        m_attr.exclude_kernel = 1;
        m_attr.exclude_hv = 0;
        m_attr.exclude_guest = 0;
        m_attr.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
        m_attr.sample_type =
            PERF_SAMPLE_TID | PERF_SAMPLE_TIME | PERF_SAMPLE_READ | PERF_SAMPLE_CPU;

        // TODO: create getter/setter for tuning the number of wakeup_events?
        // This is only needed for the group leader, all others are ignored
        m_attr.wakeup_events = internal::WAKE_EVENT_COUNT;
    }

    pmu_event::~pmu_event()
    {
        disable();
        close();
    }

    void pmu_event::configure(const uint64_t raw_config_value)
    {
        m_attr.config = raw_config_value;

        // DE7565: When the 'in_tx_cp' bit is set the perf_event_open fails, when the CPU has HLE
        // and RTM enabled.  This was detected in the Unit Test 'pmu_publisher_valid_zero_or_one.'.
        // IN_TX and IN_TX_CP (bits 32 and 33) are not used in this implementation,
        // so mask them from the event and warn that the masking has occurred.
        if (m_attr.type == PERF_TYPE_RAW && m_attr.config > UINT32_MAX)
        {
            std::cerr
                << "WARNING: The RAW Event has bits above bit 31 set. Those bits are being masked!"
                << std::endl;
            m_attr.config &= UINT32_MAX;
        }
    }

    void pmu_event::sample_period(const uint64_t period)
    {
        m_attr.sample_period = period;
    }

    bool pmu_event::open(const pid_t pid,
        const int32_t cpu,
        const int32_t group_fd,
        const uint64_t flags)
    {
        if (m_fd == internal::IO_ERROR)
        {
            m_fd = syscall(__NR_perf_event_open, &m_attr, pid, cpu, group_fd, flags);
            if (m_fd != internal::IO_ERROR)
            {
                // Create ID for event
                // TODO: Is this needed?
                int ret = ioctl(m_fd, PERF_EVENT_IOC_ID, &m_id);
                if (ret == internal::IO_ERROR)
                {
#ifdef DEBUG
                    std::cerr << __func__ << " Error creating Event ID : " << std::strerror(errno)
                              << std::endl;
#endif
                    ::close(m_fd);
                    m_fd = internal::IO_ERROR;
                }
            }
#ifdef DEBUG
            else
            {
                std::cerr << __func__ << " Error Opening Event : " << std::strerror(errno)
                          << std::endl;
            }
#endif
            _debug_print(__func__);
        }
        return (m_fd != internal::IO_ERROR);
    }

    bool pmu_event::enable() const
    {
        int ret = internal::IO_ERROR;
        if (m_fd != internal::IO_ERROR)
        {
            if (is_leader())
            {
                ret = ioctl(m_fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
                if (ret != internal::IO_ERROR)
                {
                    ret = ioctl(m_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
#ifdef DEBUG
                    if (ret == internal::IO_ERROR)
                    {
                        std::cerr << __func__ << "() Event ENABLE failed : " << std::strerror(errno)
                                  << std::endl;
                    }
#endif
                }
#ifdef DEBUG
                else
                {
                    std::cerr << __func__ << "() Event RESET Failed : " << std::strerror(errno)
                              << std::endl;
                }
#endif
            }
        }
        else
        {
#ifdef DEBUG
            std::cerr << __func__ << " Error Invalid File Descriptor " << std::endl;
#endif
            return false;
        }
        return (ret != internal::IO_ERROR);
    }

    void pmu_event::disable() const
    {
        if (m_fd != internal::IO_ERROR)
        {
            int ret = ioctl(m_fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
            if (ret != internal::IO_ERROR)
            {
                ret = ioctl(m_fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
#ifdef DEBUG
                if (ret == internal::IO_ERROR)
                {
                    std::cerr << __func__ << "() Event ENABLE failed : " << std::strerror(errno)
                              << std::endl;
                }
#endif
            }
#ifdef DEBUG
            else
            {
                std::cerr << __func__ << "() Event RESET Failed : " << std::strerror(errno)
                          << std::endl;
            }
#endif
        }
    }

    void pmu_event::close()
    {
        if (m_fd != internal::IO_ERROR)
        {
            ::close(m_fd);
            m_fd = internal::IO_ERROR;
        }
    }
}  // namespace bit_shovel_plugins
