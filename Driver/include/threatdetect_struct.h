/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   threatdetect_struct.h
**
********************************************************************************
*/

#ifndef __THREATDETECT_STRUCT_H__
#define __THREATDETECT_STRUCT_H__

#include "threatdetect_types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

    /*
     * @macro TD_VERSION_NODE_S
     * @brief
     * This structure supports versioning. The field major indicates the major version,
     * minor indicates the minor version and api indicates the api version for the current
     * threat detect build. This structure is initialized at the time when the driver is loaded.
     */
    typedef struct TD_VERSION_NODE_S TD_VERSION_NODE;
    typedef TD_VERSION_NODE* TD_VERSION;

    struct TD_VERSION_NODE_S
    {
        union
        {
            uint32_t td_version;
            struct
            {
                uint32_t major : 8;
                uint32_t minor : 8;
                uint32_t api : 8;
                uint32_t update : 8;
            } s1;
        } u1;
    };

#define TD_VERSION_NODE_version(version) (version)->u1.td_version
#define TD_VERSION_NODE_major(version) (version)->u1.s1.major
#define TD_VERSION_NODE_minor(version) (version)->u1.s1.minor
#define TD_VERSION_NODE_api(version) (version)->u1.s1.api
#define TD_VERSION_NODE_update(version) (version)->u1.s1.update

    typedef struct PMU_CONFIGURATION_NODE_S PMU_CONFIGURATION_NODE;
    typedef PMU_CONFIGURATION_NODE* PMU_CONFIGURATION;

    struct PMU_CONFIGURATION_NODE_S
    {
        uint64_t eventSel[4];
        uint32_t samplesPerInterrupt[4];
        uint64_t offCore[2];
        uint32_t samplesPerInterruptFixed[3];
        DRV_BOOL lbr_enable;                 // enable capturing LBR data
        uint64_t lbr_select;                 // value to be programmed into MSR_LBR_SELECT
        uint32_t num_lbr_entries_requested;  // 0 - full LBR stack
        // only report interrupt where the previous interrupt was also the same pid and cpu
        DRV_BOOL bleed_prevention_enabled;
    };

#define FULL_LBR_STACK 0

    /* platform config for START IOCTL */
    typedef struct _PLATFORM_CONFIGURATION
    {
        PMU_CONFIGURATION_NODE pmu_config;
    } PLATFORM_CONFIGURATION;

#define DRV_CONFIG_num_lbrs(cfg) (cfg)->num_lbrs

#define MAX_PROCESS 200

#define MAX_APP_RECORDS 1200
#define APP_RECORD_SEND_THRESHOLD 600
#define APP_RECORD_MAX_SEND_LATENCY 1000000
#define APP_UKAM_VERSION 3

#define MAX_APP_PROG_EVENTS 4
#define MAX_APP_FIXED_EVENTS 3

#define MAX_LBR_ENTRIES 32

    //
    // --------------------------------------------------
    //  Compile time flags for different telemetry types
    // --------------------------------------------------
    //
    // These are used to disable certain features entirely
    // for performance testing.
    //

#define ENABLE_PROG_COUNTERS
#define ENABLE_FRAME_DATA
#define ENABLE_FIXED_COUNTERS
//#define ENABLE_PEBS_DATA
#define ENABLE_LBR_DATA

    //
    // Structure for setting up the event which driver sets once having process data in the shared
    // mem
    //

    typedef enum
    {
        TDT_DRIVER_STATUS_SUCCESS = 0,
        TDT_DRIVER_STATUS_OVERFLOW = 1
    } tdtsdk_status_codes;

#ifdef ENABLE_PEBS_DATA
    typedef struct
    {
        uint64_t data_address;
        uint64_t instruction_address;
    } tdtsdk_pebs_record_t;
#endif  // ENABLE_PEBS_DATA

    typedef struct
    {
        uint8_t is_64_bit;
        uint64_t instruction_address;
    } tdtsdk_frame_trap_info_t;

    typedef struct
    {
        uint32_t num_populated_entries;
        uint64_t lbr_from_address[MAX_LBR_ENTRIES];  // lbr_from_address[0] is TOS
        uint64_t lbr_to_address[MAX_LBR_ENTRIES];
        uint8_t flags[MAX_LBR_ENTRIES];
    } tdtsdk_lbr_stack_t;

    typedef struct
    {
        uint64_t timeStamp;
        uint32_t pid;
        uint32_t tid;
        uint32_t cpu;
#ifdef ENABLE_FRAME_DATA
        tdtsdk_frame_trap_info_t frame;
#endif  // ENABLE_FRAME_DATA
#ifdef ENABLE_PROG_COUNTERS
        uint32_t events_data[MAX_APP_PROG_EVENTS];
#endif  // ENABLE_PROG_COUNTERS
#ifdef ENABLE_FIXED_COUNTERS
        uint32_t fixed_data[MAX_APP_FIXED_EVENTS];
#endif  // ENABLE_FIXED_COUNTERS
#ifdef ENABLE_PEBS_DATA
        tdtsdk_pebs_record_t pebs_data;
#endif  //#ifdef ENABLE_PEBS_DATA
#ifdef ENABLE_LBR_DATA
        tdtsdk_lbr_stack_t lbr_stack;
#endif  // ENABLE_LBR_DATA
    } tdtsdk_pmi_record_t;

    /**
     * Structure for the pinned user memory
     */
    typedef struct
    {
        // state tracking for ring buffer
        volatile long last_flushed;
        volatile long next_write;
        long next_read;

        // status code to report error or warning conditions (not currently used)
        // uint32_t status;

        // ring buffer
        tdtsdk_pmi_record_t records[MAX_APP_RECORDS];
    } tdtsdk_shared_mem_t;

    /**
     * Structure passed between user space and the driver in the IOCTL.
     */
    typedef struct
    {
        uint64_t buffer;       // Address of User/Kernel Accessible Memory buffer
        uint64_t buffer_size;  // Size of User/Kernel Accessible Memory buffer
        uint64_t version;      // Version of data communicated.
                               // Increment each time a major change is made.
    } tdtsdk_ioctl_sharem_mem_data_t;

#if defined(__cplusplus)
}
#endif

#endif
