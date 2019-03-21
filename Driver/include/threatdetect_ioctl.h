/****
 * -------------------------------------------------------------------------
 *               INTEL CORPORATION PROPRIETARY INFORMATION
 *  This software is supplied under the terms of the accompanying license
 *  agreement or nondisclosure agreement with Intel Corporation and may not
 *  be copied or disclosed except in accordance with the terms of that
 *  agreement.
 *        Copyright(C) 2018 Intel Corporation.  All Rights Reserved.
 * -------------------------------------------------------------------------
 ****/

#ifndef _SCPMUDRV_IOCTL_H_
#define _SCPMUDRV_IOCTL_H_

#if defined(__cplusplus)
extern "C"
{
#endif

// Side Channel Driver Operation defines
//
#define DRV_OPERATION_START 1
#define DRV_OPERATION_STOP 2
#define DRV_OPERATION_PAUSE 3
#define DRV_OPERATION_RESUME 4
#define DRV_OPERATION_RESERVE 5
#define DRV_OPERATION_VERSION 6
#define DRV_OPERATION_GET_PROCESS_INFO 10
#define DRV_OPERATION_SET_PROCESS_FILTER 11
#define DRV_OPERATION_GET_KERNEL_LBRS 12
#define DRV_OPERATION_LBR_SNAPSHOT 13
#define DRV_OPERATION_ENABLE_LBR_CAPTURING 14
#define DRV_OPERATION_DISABLE_LBR_CAPTURING 15
#define DRV_OPERATION_GET_AESNI_RECORDS 16

#define TDPMUDRV_IOCTL_ENCODE_CODE(x, y) (U64)(((U64)x << 32) | (U64)y)
#define TDPMUDRV_IOCTL_GET_TARGET_CODE(x) (U32)(x & 0x00000000ffffffff)
#define TDPMUDRV_IOCTL_GET_HOST_CODE(x) (U32)((x >> 32) & 0x00000000ffffffff)

    // IOCTL_SETUP
    //

#if defined(DRV_OS_WINDOWS)

//
// NtDeviceIoControlFile IoControlCode values for this device.
//
// Warning:  Remember that the low two bits of the code specify how the
//           buffers are passed to the driver!
//
// 16 bit device type. 12 bit function codes
#    define TDPMUDRV_IOCTL_DEVICE_TYPE 0xA000U  // values 0-32768 reserved for Microsoft
#    define TDPMUDRV_IOCTL_FUNCTION 0x0A00U     // values 0-2047  reserved for Microsoft

//
// Basic CTL CODE macro to reduce typographical errors
// Use for FILE_READ_ACCESS
//
#    define TDPMUDRV_CTL_READ_CODE(x)        \
        CTL_CODE(TDPMUDRV_IOCTL_DEVICE_TYPE, \
            TDPMUDRV_IOCTL_FUNCTION + (x),   \
            METHOD_BUFFERED,                 \
            FILE_READ_ACCESS)

//
// Basic CTL CODE macro to reduce typographical errors
// Use for FILE_WRITE_ACCESS (Ioctls defined with this macro will require Admin privileges to be
// used).
//
#    define LWPMUDRV_CTL_WRITE_CODE(x)       \
        CTL_CODE(LWPMUDRV_IOCTL_DEVICE_TYPE, \
            LWPMUDRV_IOCTL_FUNCTION + (x),   \
            METHOD_BUFFERED,                 \
            FILE_WRITE_ACCESS)

#    define TDPMUDRV_IOCTL_START TDPMUDRV_CTL_READ_CODE(DRV_OPERATION_START)
#    define TDPMUDRV_IOCTL_STOP TDPMUDRV_CTL_READ_CODE(DRV_OPERATION_STOP)
#    define TDPMUDRV_IOCTL_PAUSE TDPMUDRV_CTL_READ_CODE(DRV_OPERATION_PAUSE)
#    define TDPMUDRV_IOCTL_RESUME TDPMUDRV_CTL_READ_CODE(DRV_OPERATION_RESUME)
#    define TDPMUDRV_IOCTL_RESERVE TDPMUDRV_CTL_READ_CODE(DRV_OPERATION_RESERVE)
#    define TDPMUDRV_IOCTL_VERSION TDPMUDRV_CTL_READ_CODE(DRV_OPERATION_VERSION)
#    define TDPMUDRV_IOCTL_GET_PROCESS_DATA TDPMUDRV_CTL_WRITE_CODE(DRV_OPERATION_GET_PROCESS_INFO)
#    define TDPMUDRV_IOCTL_SET_PROCESS_FILTER \
        TDPMUDRV_CTL_WRITE_CODE(DRV_OPERATION_SET_PROCESS_FILTER)
#    define TDPMUDRV_IOCTL_GET_KERNEL_LBRS TDPMUDRV_CTL_WRITE_CODE(DRV_OPERATION_GET_KERNEL_LBRS)
#    define TDPMUDRV_IOCTL_LBR_SNAPSHOT TDPMUDRV_CTL_WRITE_CODE(DRV_OPERATION_LBR_SNAPSHOT)
#    define TDPMUDRV_IOCTL_ENABLE_LBR_CAPTURING \
        TDPMUDRV_CTL_WRITE_CODE(DRV_OPERATION_ENABLE_LBR_CAPTURING)
#    define TDPMUDRV_IOCTL_DISABLE_LBR_CAPTURING \
        TDPMUDRV_CTL_WRITE_CODE(DRV_OPERATION_DISABLE_LBR_CAPTURING)
    //#define TDPMUDRV_IOCTL_GET_AESNI_RECORDS
    // TDPMUDRV_CTL_WRITE_CODE(DRV_OPERATION_GET_AESNI_RECORDS)

    // IOCTL_ARGS
    typedef struct IOCTL_ARGS_NODE_S IOCTL_ARGS_NODE;
    typedef IOCTL_ARGS_NODE* IOCTL_ARGS;
    struct IOCTL_ARGS_NODE_S
    {
        U64 r_len;
        char* r_buf;
        U64 w_len;
        char* w_buf;
    };

#elif defined(SEP_ESX)
// IOCTL_ARGS
typedef struct IOCTL_ARGS_NODE_S IOCTL_ARGS_NODE;
typedef IOCTL_ARGS_NODE* IOCTL_ARGS;
struct IOCTL_ARGS_NODE_S
{
    U32 cmd;
    U64 r_len;
    char* r_buf;
    U64 w_len;
    char* w_buf;
};

typedef struct CPU_ARGS_NODE_S CPU_ARGS_NODE;
typedef CPU_ARGS_NODE* CPU_ARGS;
struct CPU_ARGS_NODE_S
{
    U64 r_len;
    char* r_buf;
    U32 command;
    U32 CPU_ID;
    U32 BUCKET_ID;
};

// IOCTL_SETUP
#    define TDPMU_IOC_MAGIC 98

// Task file Opcodes.
// keeping the definitions as IOCTL but in MAC OSX
// these are really OpCodes consumed by Execute command.
#    define TDPMUDRV_IOCTL_START 1
#    define TDPMUDRV_IOCTL_STOP 2

#elif defined(DRV_OS_LINUX) || defined(DRV_OS_SOLARIS) || defined(DRV_OS_ANDROID)
// IOCTL_ARGS
typedef struct IOCTL_ARGS_NODE_S IOCTL_ARGS_NODE;
typedef IOCTL_ARGS_NODE* IOCTL_ARGS;
#    if defined(DRV_EM64T)
struct IOCTL_ARGS_NODE_S
{
    U64 r_len;
    char* r_buf;
    U64 w_len;
    char* w_buf;
};
#    endif
#    if defined(DRV_IA32)
struct IOCTL_ARGS_NODE_S
{
    U64 r_len;
    char* r_buf;
    char* r_reserved;
    U64 w_len;
    char* w_buf;
    char* w_reserved;
};
#    endif

// COMPAT IOCTL_ARGS
#    if defined(CONFIG_COMPAT) && defined(DRV_EM64T)
typedef struct IOCTL_COMPAT_ARGS_NODE_S IOCTL_COMPAT_ARGS_NODE;
typedef IOCTL_COMPAT_ARGS_NODE* IOCTL_COMPAT_ARGS;
struct IOCTL_COMPAT_ARGS_NODE_S
{
    U64 r_len;
    compat_uptr_t r_buf;
    U64 w_len;
    compat_uptr_t w_buf;
};
#    endif

// COMPAT IOCTL_SETUP
//
#    define TDPMU_IOC_MAGIC 98

#    if defined(CONFIG_COMPAT) && defined(DRV_EM64T)
#        define TDPMUDRV_IOCTL_COMPAT_RESERVE \
            _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_RESERVE, compat_uptr_t)
#        define TDPMUDRV_IOCTL_COMPAT_VERSION \
            _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_VERSION, compat_uptr_t)
#    endif

#    define TDPMUDRV_IOCTL_START _IOW(TDPMU_IOC_MAGIC, DRV_OPERATION_START, IOCTL_ARGS)
#    define TDPMUDRV_IOCTL_STOP _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_STOP)
#    define TDPMUDRV_IOCTL_PAUSE _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_PAUSE)
#    define TDPMUDRV_IOCTL_RESUME _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_RESUME)
#    define TDPMUDRV_IOCTL_RESERVE _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_RESERVE, IOCTL_ARGS)
#    define TDPMUDRV_IOCTL_VERSION _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_VERSION, IOCTL_ARGS)
#    define TDPMUDRV_IOCTL_GET_PROCESS_DATA \
        _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_GET_PROCESS_INFO, IOCTL_ARGS)
#    define TDPMUDRV_IOCTL_SET_PROCESS_FILTER \
        _IOW(TDPMU_IOC_MAGIC, DRV_OPERATION_SET_PROCESS_FILTER, IOCTL_ARGS)
#    define TDPMUDRV_IOCTL_GET_KERNEL_LBRS \
        _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_GET_KERNEL_LBRS, IOCTL_ARGS)
#    define TDPMUDRV_IOCTL_LBR_SNAPSHOT _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_LBR_SNAPSHOT)
#    define TDPMUDRV_IOCTL_ENABLE_LBR_CAPTURING \
        _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_ENABLE_LBR_CAPTURING)
#    define TDPMUDRV_IOCTL_DISABLE_LBR_CAPTURING \
        _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_DISABLE_LBR_CAPTURING)
#    define TDPMUDRV_IOCTL_GET_AESNI_RECORDS \
        _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_GET_AESNI_RECORDS, IOCTL_ARGS)

#elif defined(DRV_OS_FREEBSD)

// IOCTL_ARGS
typedef struct IOCTL_ARGS_NODE_S IOCTL_ARGS_NODE;
typedef IOCTL_ARGS_NODE* IOCTL_ARGS;
struct IOCTL_ARGS_NODE_S
{
    U64 r_len;
    char* r_buf;
    U64 w_len;
    char* w_buf;
};

// IOCTL_SETUP
//
#    define TDPMU_IOC_MAGIC 98

/* FreeBSD is very strict about IOR/IOW/IOWR specifications on IOCTLs.
 * Since these IOCTLs all pass down the real read/write buffer lengths
 *  and addresses inside of an IOCTL_ARGS_NODE data structure, we
 *  need to specify all of these as _IOW so that the kernel will
 *  view it as userspace passing the data to the driver, rather than
 *  the reverse.  There are also some cases where Linux is passing
 *  a smaller type than IOCTL_ARGS_NODE, even though its really
 *  passing an IOCTL_ARGS_NODE.  These needed to be fixed for FreeBSD.
 */

#    define TDPMUDRV_IOCTL_START _IOW(TDPMU_IOC_MAGIC, DRV_OPERATION_START, IOCTL_ARGS)
#    define TDPMUDRV_IOCTL_STOP _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_STOP)
#    define TDPMUDRV_IOCTL_PAUSE _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_PAUSE)
#    define TDPMUDRV_IOCTL_RESUME _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_RESUME)
#    define TDPMUDRV_IOCTL_RESERVE _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_RESERVE, IOCTL_ARGS_NODE)
#    define TDPMUDRV_IOCTL_VERSION _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_VERSION, IOCTL_ARGS_NODE)
#    define TDPMUDRV_IOCTL_GET_PROCESS_DATA \
        _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_GET_PROCESS_INFO, IOCTL_ARGS_NODE)
#    define TDPMUDRV_IOCTL_SET_PROCESS_FILTER \
        _IOW(TDPMU_IOC_MAGIC, DRV_OPERATION_SET_PROCESS_FILTER, IOCTL_ARGS_NODE)
#    define TDPMUDRV_IOCTL_GET_KERNEL_LBRS \
        _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_GET_KERNEL_LBRS, IOCTL_ARGS_NODE)
#    define TDPMUDRV_IOCTL_LBR_SNAPSHOT _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_LBR_SNAPSHOT)
#    define TDPMUDRV_IOCTL_ENABLE_LBR_CAPTURING \
        _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_ENABLE_LBR_CAPTURING)
#    define TDPMUDRV_IOCTL_DISABLE_LBR_CAPTURING \
        _IO(TDPMU_IOC_MAGIC, DRV_OPERATION_DISABLE_LBR_CAPTURING)
#    define TDPMUDRV_IOCTL_GET_AESNI_RECORDS \
        _IOR(TDPMU_IOC_MAGIC, DRV_OPERATION_GET_AESNI_RECORDS, IOCTL_ARGS_NODE)

#elif defined(DRV_OS_MAC)

// IOCTL_ARGS
typedef struct IOCTL_ARGS_NODE_S IOCTL_ARGS_NODE;
typedef IOCTL_ARGS_NODE* IOCTL_ARGS;
struct IOCTL_ARGS_NODE_S
{
    U64 r_len;
    char* r_buf;
    U64 w_len;
    char* w_buf;
    U32 command;
};

typedef struct CPU_ARGS_NODE_S CPU_ARGS_NODE;
typedef CPU_ARGS_NODE* CPU_ARGS;
struct CPU_ARGS_NODE_S
{
    U64 r_len;
    char* r_buf;
    U32 command;
    U32 CPU_ID;
    U32 BUCKET_ID;
};

// IOCTL_SETUP
//
#    define TDPMU_IOC_MAGIC 98

// Task file Opcodes.
// keeping the definitions as IOCTL but in MAC OSX
// these are really OpCodes consumed by Execute command.
#    define TDPMUDRV_IOCTL_START DRV_OPERATION_START
#    define TDPMUDRV_IOCTL_STOP DRV_OPERATION_STOP
#    define TDPMUDRV_IOCTL_PAUSE DRV_OPERATION_PAUSE
#    define TDPMUDRV_IOCTL_RESUME DRV_OPERATION_RESUME
#    define TDPMUDRV_IOCTL_RESERVE DRV_OPERATION_RESERVE
#    define TDPMUDRV_IOCTL_VERSION DRV_OPERATION_VERSION
#    define TDPMUDRV_IOCTL_GET_PROCCESS_INFO DRV_OPERATION_GET_PROCESS_INFO
#    define TDPMUDRV_IOCTL_SET_PROCCESS_FILTER DRV_OPERATION_SET_PROCESS_FILTER
#    define TDPMUDRV_IOCTL_GET_KERNEL_LBRS DRV_OPERATION_GET_KERNEL_LBRS
#    define TDPMUDRV_IOCTL_LBR_SNAPSHOT DRV_OPERATION_LBR_SNAPSHOT
#    define TDPMUDRV_IOCTL_ENABLE_LBR_CAPTURING DRV_OPERATION_ENABLE_LBR_CAPTURING
#    define TDPMUDRV_IOCTL_DISABLE_LBR_CAPTURING DRV_OPERATION_DISABLE_LBR_CAPTURING

// This is only for MAC OSX
#    define TDPMUDRV_IOCTL_GET_ASLR_OFFSET 997  // this may not need
#    define TDPMUDRV_IOCTL_SET_OSX_VERSION 998
#    define TDPMUDRV_IOCTL_PROVIDE_FUNCTION_PTRS 999

#else
#    error "unknown OS in lwpmudrv_ioctl.h"
#endif

#if defined(__cplusplus)
}
#endif

#endif
