;********************************************************************************
;    Intel Architecture Group
;    Copyright (C) 2019 Intel Corporation
;********************************************************************************
;
;    INTEL CONFIDENTIAL
;    This file, software, or program is supplied under the terms of a
;    license agreement and/or nondisclosure agreement with Intel Corporation
;    and may not be copied or disclosed except in accordance with the
;    terms of that agreement.  This file, software, or program contains
;    copyrighted material and/or trade secret information of Intel
;    Corporation, and must be treated as such.  Intel reserves all rights
;    in this material, except as the license agreement or nondisclosure
;    agreement specifically indicate.
;
;    All rights reserved.  No part of this program or publication
;    may be reproduced, transmitted, transcribed, stored in a
;    retrieval system, or translated into any language or computer
;    language, in any form or by any means, electronic, mechanical,
;    magnetic, optical, chemical, manual, or otherwise, without
;    the prior written permission of Intel Corporation.
;
;    Intel makes no warranty of any kind regarding this code.  This code
;    is provided on an "As Is" basis and Intel will not provide any support,
;    assistance, installation, training or other services.  Intel does not
;    provide any updates, enhancements or extensions.  Intel specifically
;    disclaims any warranty of merchantability, noninfringement, fitness
;    for any particular purpose, or any other warranty.
;
;    Intel disclaims all liability, including liability for infringement
;    of any proprietary rights, relating to use of the code.  No license,
;    express or implied, by estoppel or otherwise, to any intellectual
;    property rights is granted herein.
;
;********************************************************************************
;
;    @file   telemetry_collect_tdtlib.profile
;
;    @brief  This profile is used to collect telemetry data.
;
;
;********************************************************************************

; This file is in INFO format supported by boost::property_tree
; To output a set of property value as JSON array their keys need to be empty ""
; for example set_of_values {
;               "" "value1"
;               "" "value2"
;             }
; will be output in JSON as { "set_of_values": ["value1", "value2"] }
;
; Properties that neeed to be exported as read only should be placed under export.read_only.
; Properties that neeed to be exported as read/write should be placed under export.read_write.
; For phase 1 TDT Library deliverable to preserve backward compatibility exported properties should
; also be available under root.
; TODO: enforce export schema for all profiles and update plugins.

telemetry_collect_tdtlib ; should be same as profile file name without extension
{
    description "Collects telemetry data"
    author "Intel (C) Corp."
    date "2019-02-16" ; yyyy-mm-dd

    ; plugins used by the profile
    plugins
    {
        "" pmu_publisher
        "" process_monitor
        "" csv_telemetry_recorder
    }
}


;
; PLUGINS SETTINGS
;


; scans OS process list and provide process names from pids
; optionally sends a list of pids to driver that should be ignored/not measured
process_monitor
{
    ; String to use when process name is not available
    unknown_process_name "Process"

    ; throttling rate for scanning OS process list
    sampling_rate_in_ms   500

    ; if set to false, process names will be monitored but no filter data
    ; will be sent to the driver
    enable_process_filter true

    ; Array of executable path string prefixes to be filtered by driver
    process_filter {

        windows {
            "" "C:\\windows\\system32\\"
            "" "C:\\windows\\explorer.exe"
        }

        linux {
            "" "/usr/sbin/"
        }
    }
}

; controls driver PMU configuration and receives hardware telemetry data to be used by the normalizer
pmu_publisher
{
    ; array of event configuration strings to control PMU settings
    event_strings
    {
        ; For general programmable counters, you can use the pcm format, one per line.
        ; The counters will be filled in order, 0-3, so be sure to keep that in mind
        ; for events which must be on counters 0-1 such as offcore events.
        ;
        ; Example: cpu/umask=0x06,event=0xA3,name=CYCLE_ACTIVITY.STALLS_L3_MISS,cmask=6,int=1000000/
        ;
        ; Extra support has been added to add interrupting on a counter using the int=# flag.
        ;
        ; Example: cpu/umask=0x00,event=0xC0,name=INST_RETIRED.ANY_P/
        ;
        ; The fixed counters are always recording, but you can choose to interrupt on them
        ; by putting cpu/fixed=<0-3 counter index>,int=<number of counted events to trigger an interrupt>
        ;
        ; Example: cpu/fixed=0,int=100000/
        ; The above would generate an interrupt for every 100,000 instructions executed.
        ;

        "0" "cpu/umask=0x06,event=0xA3,name=CYCLE_ACTIVITY.STALLS_L3_MISS,cmask=6,int=1000000/" ; event0
        "1" "cpu/umask=0x01,event=0x3C,name=CPU_CLK_THREAD_UNHALTED.REF_XCLK/" ; event1
        "2" "cpu/umask=0x01,event=0xA6,name=EXE_ACTIVITY.EXE_BOUND_0_PORTS/" ; event2
        "3" "cpu/umask=0x00,event=0xC0,name=INST_RETIRED.ANY_P/" ; event3
    }
}

; plugin to write raw telemetry output to a file
csv_telemetry_recorder
{
    ; name of file to have output written to
    file_name "telemetry_output.csv"

    ; separator character for fields (new line = new record)
    separator ","

    ; Adding a blank line between data batches enables easier offline replay
    enable_blank_line_between_data_batches true

    ; For debugging, record time latency for each record
    enable_record_latency_logging false
}

export
{
    read_only
    {
    }

    read_write
    {
        ; scans OS process list and provide process names from pids
        ; optionally sends a list of pids to driver that should be ignored/not measured
        process_monitor
        {
            ; String to use when process name is not available
            unknown_process_name "Process"

            ; throttling rate for scanning OS process list
            sampling_rate_in_ms   500

            ; if set to false, process names will be monitored but no filter data
            ; will be sent to the driver
            enable_process_filter true

            ; Array of executable path string prefixes to be filtered by driver
            process_filter {

                windows {
                    "" "C:\\windows\\system32\\"
                    "" "C:\\windows\\explorer.exe"
                }

                linux {
                    "" "/usr/sbin/"
                }
            }
        }

        csv_telemetry_recorder
        {
            ; name of file to have output written to
            file_name "telemetry_output.csv"

            ; separator character for fields (new line = new record)
            separator ","

            ; Adding a blank line between data batches enables easier offline replay
            enable_blank_line_between_data_batches true

            ; For debugging, record time latency for each record
            enable_record_latency_logging false
        }

        ; controls driver PMU configuration and receives hardware telemetry data to be used by the normalizer
        pmu_publisher
        {
            ; array of event configuration strings to control PMU settings
            event_strings
            {
                ; For general programmable counters, you can use the pcm format, one per line.
                ; The counters will be filled in order, 0-3, so be sure to keep that in mind
                ; for events which must be on counters 0-1 such as offcore events.
                ;
                ; Example: cpu/umask=0x06,event=0xA3,name=CYCLE_ACTIVITY.STALLS_L3_MISS,cmask=6,int=1000000/
                ;
                ; Extra support has been added to add interrupting on a counter using the int=# flag.
                ;
                ; Example: cpu/umask=0x00,event=0xC0,name=INST_RETIRED.ANY_P/
                ;
                ; The fixed counters are always recording, but you can choose to interrupt on them
                ; by putting cpu/fixed=<0-3 counter index>,int=<number of counted events to trigger an interrupt>
                ;
                ; Example: cpu/fixed=0,int=100000/
                ; The above would generate an interrupt for every 100,000 instructions executed.
                ;

                "0" "cpu/umask=0x06,event=0xA3,name=CYCLE_ACTIVITY.STALLS_L3_MISS,cmask=6,int=1000000/" ; event0
                "1" "cpu/umask=0x01,event=0x3C,name=CPU_CLK_THREAD_UNHALTED.REF_XCLK/" ; event1
                "2" "cpu/umask=0x01,event=0xA6,name=EXE_ACTIVITY.EXE_BOUND_0_PORTS/" ; event2
                "3" "cpu/umask=0x00,event=0xC0,name=INST_RETIRED.ANY_P/" ; event3
            }
        }
    }
}
