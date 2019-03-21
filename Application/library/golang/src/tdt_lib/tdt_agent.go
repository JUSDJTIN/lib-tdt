/*
********************************************************************************
**    Intel Architecture Group
**    Copyright (C) 2018-2019 Intel Corporation
********************************************************************************
**
**    INTEL CONFIDENTIAL
**    This file, software, or program is supplied under the terms of a
**    license agreement and/or nondisclosure agreement with Intel Corporation
**    and may not be copied or disclosed except in accordance with the
**    terms of that agreement.  This file, software, or program contains
**    copyrighted material and/or trade secret information of Intel
**    Corporation, and must be treated as such.  Intel reserves all rights
**    in this material, except as the license agreement or nondisclosure
**    agreement specifically indicate.
**
**    All rights reserved.  No part of this program or publication
**    may be reproduced, transmitted, transcribed, stored in a
**    retrieval system, or translated into any language or computer
**    language, in any form or by any means, electronic, mechanical,
**    magnetic, optical, chemical, manual, or otherwise, without
**    the prior written permission of Intel Corporation.
**
**    Intel makes no warranty of any kind regarding this code.  This code
**    is provided on an "As Is" basis and Intel will not provide any support,
**    assistance, installation, training or other services.  Intel does not
**    provide any updates, enhancements or extensions.  Intel specifically
**    disclaims any warranty of merchantability, noninfringement, fitness
**    for any particular purpose, or any other warranty.
**
**    Intel disclaims all liability, including liability for infringement
**    of any proprietary rights, relating to use of the code.  No license,
**    express or implied, by estoppel or otherwise, to any intellectual
**    property rights is granted herein.
**/
/********************************************************************************
**
**    @file   tdt_agent.go
**
**    @brief  GO interface for Threat Detection Technology Library.
**
**
********************************************************************************
*/

// Package tdt_lib provides Go interface to the native C++ Threat Detection
// Technology Library.
package tdt_lib

/*
// // preanble for cgo. see https://golang.org/cmd/cgo/
#cgo CFLAGS: -std=c99 -I../../../ -I./
#cgo LDFLAGS: -L../../../../../build/bins/x64/ -L../../../../../build/bins/x64/Debug -L../../../../../build/bins/x64/Release -L./ -Wl,-rpath=./ -ltdt
#include <stdlib.h>
#include "tdt_agent.h"

tdt_return_code SetNotificationCallback(agent_handle hagent, const long long context);
*/
import "C"
import "unsafe"
import "fmt"
import "sync"

// The return codes from the TDT Library APIs.
// Should be kept up to date with C API tdt_return code
type tdt_return_code uint

const (
    TDT_ERROR_SUCCESS tdt_return_code = iota
    TDT_ERROR_NULL_PARAM
    TDT_ERROR_INVALID_PARAM
    TDT_ERROR_OUT_OF_MEMORY
    TDT_ERROR_INTERNAL
    TDT_ERROR_INSUFFICIENT_BUFFER_SIZE
    TDT_ERROR_NOT_IMPLEMENTED
    TDT_ERROR_STARTUP_FAILURE
    TDT_ERROR_INVALID_PLUGIN
    TDT_ERROR_INVALID_CONFIG
    TDT_ERROR_NO_EXECUTION
    TDT_ERROR_AGENT_RUNNING
    TDT_ERROR_AGENT_NOT_RUNNING
    TDT_ERROR_AGENT_ABORTED
    TDT_ERROR_SIGNVERIFY_FAILED
    TDT_ERROR_NO_PROFILES_AVAILABLE
    TDT_WARNING_NOT_ALL_PROFILES_LOADED
    TDT_ERROR_PROFILES_DIR_NOT_EXISTS
    TDT_ERROR_AGENT_UNABLE_TO_STOP
    TDT_ERROR_PIPELINE_NOT_FUNCTIONAL

    TDT_ERROR_MAX
)

// The protocol format to use when using the TDT Library APIs.
type tdt_protocol_format uint

const (
    TDT_PROTO_FORMAT_JSON tdt_protocol_format = iota
    TDT_PROTO_FORMAT_XML

    TDT_PROTO_FORMAT_MAX
)

// An internal structure to store a TDT agent instance.
type tdt_agent struct {
    h C.agent_handle
    f func(uint64, string)
    c uint64
}

// To make concurrency safe
var callbackMutex sync.Mutex

// Map to associate agent instance with context
var callbackFuncs = make(map[uint64]*tdt_agent)

// The following tests for overflow at compile time to make sure Go interface matches the C API interface
var static_assert1 = (TDT_ERROR_MAX - C.TDT_ERROR_MAX) - (C.TDT_ERROR_MAX - TDT_ERROR_MAX)
var static_assert2 = (TDT_PROTO_FORMAT_MAX - C.TDT_PROTO_FORMAT_MAX) - (C.TDT_PROTO_FORMAT_MAX - TDT_PROTO_FORMAT_MAX)

// Factory method to get an instance of TDT agent.
func GetTDTAgentInstance() *tdt_agent {
    agent := &tdt_agent {h: nil}
    return agent
}

// Constructs and initializes an instance of TDT agent to use the
// specified protocol format (e.g. JSON) for communication.
// Returns nil on success or any encountered error.
func (agent *tdt_agent) Construct(proto_fmt tdt_protocol_format) error {
    if agent.h == nil {
        agent_h := C.construct_agent(C.tdt_protocol_format(proto_fmt))
        if agent_h != nil {
           agent.h = agent_h
           return nil
        }
        return fmt.Errorf("invalid arg or out of memory")
    }
    return fmt.Errorf("invalid handle")
}

// Destructs an instance of TDT agent.
// Returns nil on success or any encountered error.
func (agent *tdt_agent) Destruct() error {
    if agent.h != nil {
        C.destruct_agent(agent.h)
        agent.h = nil
        return nil
    }
    return fmt.Errorf("invalid handle")
}

// Starts the threat detection for all currently configured threat profiles.
// Returns TDT_ERROR_SUCCESS or an error code.
func (agent *tdt_agent) Start() tdt_return_code {
    if agent.h != nil {
        return tdt_return_code(C.start(agent.h))
    }
    return TDT_ERROR_NULL_PARAM
}

// Stops threat detection for all currently configured profiles or a specified profile.
// If opt_profile is empty then stops detection for all profiles.
// Returns TDT_ERROR_SUCCESS or an error code.
func (agent *tdt_agent) Stop(opt_profile string) tdt_return_code {
    if agent.h != nil {
        var opt_ps *C.char
        var opt_ps_len C.size_t
        if opt_profile != "" {
            opt_ps = C.CString(opt_profile)
            opt_ps_len = (C.size_t)(len(opt_profile))
        }
        return tdt_return_code(C.stop(agent.h, opt_ps, opt_ps_len))
    }
    return TDT_ERROR_NULL_PARAM
}

// Discovers capabilities and threat profiles currently supported.
// On success it returns the capabilities and profiles in the protocol format
// specified during construction of the TDT agent instance and
// TDT_ERROR_SUCCESS on success. On TDT_ERROR_NULL_PARAM it returns an empty string.
// TDT_WARNING_NOT_ALL_PROFILES_LOADED if partial list of discovered profiles is returned.
// minimum versiona and build information is returned on other error codes.
func (agent *tdt_agent) Discover() (string, tdt_return_code) {
    ret_code := TDT_ERROR_NULL_PARAM
    if agent.h != nil {
        var caps_len C.size_t
        ret_code = tdt_return_code(C.discover(agent.h, nil, 0, &caps_len))
        if ret_code == TDT_ERROR_INSUFFICIENT_BUFFER_SIZE {
            buffer := C.malloc(C.size_t(caps_len)) // guarantees never to return nil
            ret_code = tdt_return_code(C.discover(agent.h, (*C.char)(buffer), caps_len, nil))
            gs := C.GoString((*C.char)(buffer))
            C.free(unsafe.Pointer(buffer))
            return gs, ret_code
        }
    }
    return "", ret_code
}

// Gets configuration for optional threat profile or for all currently supported threat profiles.
// On success it returns the configuration(s) in the protocol format
// specified during construction of the TDT agent instance and
// TDT_ERROR_SUCCESS on success. On TDT_ERROR_NULL_PARAM it returns an empty string.
// TDT_WARNING_NOT_ALL_PROFILES_LOADED if partial list of profiles with their configurations is returned.
// Returns empty object in protocol format specified during construction for all other error codes.
func (agent *tdt_agent) GetConfiguration(opt_profile string) (string, tdt_return_code) {
    gs := ""
    ret_code := TDT_ERROR_NULL_PARAM
    if agent.h != nil {
        var caps_len C.size_t
        var opt_ps *C.char
        var opt_ps_len C.size_t
        if opt_profile != "" {
            opt_ps = C.CString(opt_profile)
            opt_ps_len = (C.size_t)(len(opt_profile))
        }

        ret_code = tdt_return_code(C.get_configuration(agent.h, opt_ps, opt_ps_len, nil, 0, &caps_len))
        if ret_code == TDT_ERROR_INSUFFICIENT_BUFFER_SIZE {
            buffer := C.malloc(C.size_t(caps_len)) // guarantees never to return nil
            ret_code = tdt_return_code(C.get_configuration(agent.h, opt_ps, opt_ps_len, (*C.char)(buffer), caps_len, nil))
            gs = C.GoString((*C.char)(buffer))
            C.free(unsafe.Pointer(buffer))
        }
        if opt_ps != nil {
            C.free(unsafe.Pointer(opt_ps))
        }
    }
    return gs, ret_code
}

// Sets configuration for optional threat profile or for all currently supported threat profiles.
// The config string should be in the protocol format specified at construction of the TDT agent instance.
// Returns TDT_ERROR_SUCCESS or an error code.
func (agent *tdt_agent) SetConfiguration(opt_profile string, config string) tdt_return_code {
    ret_code := TDT_ERROR_NULL_PARAM
    if agent.h != nil {
        if config != "" {
            var opt_ps *C.char
            var opt_ps_len C.size_t
            if opt_profile != "" {
                opt_ps = C.CString(opt_profile)
                opt_ps_len = (C.size_t)(len(opt_profile))
            }
            cs := C.CString(config)
            cs_len := (C.size_t)(len(config))

            ret_code = tdt_return_code(C.set_configuration(agent.h, opt_ps, opt_ps_len, cs, cs_len ))
            C.free(unsafe.Pointer(cs))

            if opt_ps != nil {
                C.free(unsafe.Pointer(opt_ps))
            }
        } else {
            ret_code = TDT_ERROR_INVALID_PARAM
        }
    }
    return ret_code
}

// The Go notification callback exported to be invoked from C
// It uses the unique context to acquire correct agent from a map
// and invokes the notification function for that agent.
// see https://golang.org/cmd/cgo/ for more details about export.
//export GoNotifier
func GoNotifier(context uint64, msg string) {
    callbackMutex.Lock()
    agent := callbackFuncs[context]
    callbackMutex.Unlock()

    if agent == nil {
        panic("missing callback context")
    }
    if agent.c != context {
        panic("callback context mismatch")
    }
    if agent.f == nil {
        panic("missing callback function in context")
    }
    agent.f(context, msg)
}

// Sets the callback for notification of detected threats or runtime errors.
// context will be passed to the callback whenever it is invoked. context cannot be 0.
// Returns TDT_ERROR_SUCCESS or an error code. TDT_ERROR_INVALID_PARAM will be returned
// if context is already used by another agent. callback will be overwritten for already existing context.
func (agent *tdt_agent) SetNotifier(callback func(uint64, string), context uint64) tdt_return_code {
    ret_code := TDT_ERROR_NULL_PARAM
    if agent.h != nil && context != 0 && callback != nil {
        agent.f = callback
        agent.c = context

        // Read passing pointers section in below article.
        // https://golang.org/cmd/cgo/#hdr-Using_cgo_with_the_go_command
        /* https://github.com/golang/go/issues/12416
        Go code that wants to preserve funcs/pointers stores them into a map indexed by an int.
        Go code calls the C code, passing the int,which the C code may store freely.
        When the C code wants to call into Go, it passes the int to a Go function that looks
        in the map and makes the call. An explicit call is required to release the value from the
        map if it is no longer needed.
        */
        callbackMutex.Lock()
        any_existing_agent := callbackFuncs[context]
        if any_existing_agent == nil || any_existing_agent == agent {
            callbackFuncs[context] = agent // overwrite existing
            callbackMutex.Unlock()
            ret_code = tdt_return_code(C.SetNotificationCallback(agent.h, C.longlong(context)))
        } else {
            callbackMutex.Unlock()
            ret_code = TDT_ERROR_INVALID_PARAM
        }
    }
    return ret_code
}

// Returns the string description of error code.
func GetErrorString(err_code tdt_return_code) string {
    cs := C.GoString(C.get_error_string(C.tdt_return_code(err_code)))
    return cs
}