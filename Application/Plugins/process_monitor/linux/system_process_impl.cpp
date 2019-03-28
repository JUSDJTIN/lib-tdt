/*
********************************************************************************
**    Copyright (C) 2019 Intel Corporation
**    SPDX-License-Identifier: BSD-3-Clause
********************************************************************************
**
**    @file   system_process_impl.cpp
**
**    @brief  Get the process lists of the Linux system
**
**
********************************************************************************
*/

#ifdef DEBUG
#    include <iostream>
#endif
#include <algorithm>
#include <sstream>
#include <string>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <linux/limits.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "system_process_impl.h"

namespace bit_shovel_plugins
{
    namespace internal
    {
        /**
         * @brief Path of the proc filesystem
         */
        const boost::filesystem::path PROC_FS_PATH("/proc/");
    }  // namespace internal

    pid_t system_process_impl::current_process_id()
    {
        return getpid();
    }

    bool system_process_impl::populate_process_list(process_data_list_t& current_processes)
    {
        bool success = false;

        try
        {
            for (boost::filesystem::directory_entry& d :
                boost::filesystem::directory_iterator(internal::PROC_FS_PATH))
            {
                if (boost::filesystem::is_directory(d.path()))
                {
                    const std::string proc_dir_name = d.path().filename().string();
                    if (_is_numeric(proc_dir_name))
                    {
                        const pid_t pid = boost::lexical_cast<pid_t>(proc_dir_name);
                        success = update_single_process(pid, current_processes);
                        if (success == false)
                        {
                            break;
                        }
                    }
                }
            }
        }
        catch (const boost::bad_lexical_cast&)
        {
            success = false;
        }
        catch (const boost::filesystem::filesystem_error&)
        {
            success = false;
        }
        return success;
    }

    bool system_process_impl::update_single_process(pid_t pid, process_data_list_t& processes)
    {
        bool success = false;

        try
        {

            boost::system::error_code ec = {};

            const std::string pid_str = boost::lexical_cast<std::string>(pid);
            boost::filesystem::path proc_path(internal::PROC_FS_PATH);  //  path to /proc files
            proc_path /= pid_str;

            if (boost::filesystem::exists(proc_path, ec))
            {
                process_data_t process_data;

                const boost::filesystem::path executable_path =
                    _get_executable_path_from_proc_fs(pid);
                process_data.executable_path = executable_path.string();

                // Extract the process name from the executable path.
                process_data.name = executable_path.filename().string();

                // If the process_name was not retrieved from the executable path,
                // get it from the stat tokens after removing the parens.
                if (process_data.name.empty())
                {
                    // Read /proc/<pid>/stat to get the "process name" (1st 15 characters)
                    // See the proc man page for details.
                    proc_path = internal::PROC_FS_PATH;  // Build the path (/proc/<pid>/stat)
                    proc_path /= pid_str;
                    proc_path /= "stat";
                    std::ifstream proc_stat_strm(proc_path.string());
                    if (proc_stat_strm.is_open())
                    {
                        constexpr auto EXPECTED_STAT_TOKEN_COUNT = 44;
                        std::string line;
                        std::getline(proc_stat_strm, line);
                        std::vector<std::string> proc_stat_tokens;
                        boost::algorithm::split(
                            proc_stat_tokens, line, boost::algorithm::is_any_of(" "));
                        if (!proc_stat_tokens.empty() &&
                            proc_stat_tokens.size() >= EXPECTED_STAT_TOKEN_COUNT)
                        {
                            boost::algorithm::trim_if(
                                proc_stat_tokens[1], boost::algorithm::is_any_of("()"));
                            process_data.name = proc_stat_tokens[1];

#ifdef DEBUG_VERBOSE
                            std::cout << __func__ << "() pid = " << pid << " executable path ["
                                      << executable_path << "]"
                                      << " process name [" << process_data.name << "]" << std::endl;
#endif
                        }
                    }
                }
                success = !process_data.name.empty();

                processes[pid] = process_data;
            }
        }
        catch (const boost::bad_lexical_cast&)
        {
            success = false;
        }
        catch (const boost::filesystem::filesystem_error&)
        {
            success = false;
        }

        return success;
    }

    uint64_t system_process_impl::sampling_start_time()
    {
        uint64_t starttime = 0;
        try
        {
            const std::string pid_str = boost::lexical_cast<std::string>(getpid());
            boost::filesystem::path proc_path(internal::PROC_FS_PATH);  //  path to /proc files
            proc_path /= pid_str;

            if (boost::filesystem::exists(proc_path))
            {
                // Read /proc/<pid>/stat to get the "system time", "user time", "start time"
                // and the "process name" (1st 15 characters) if not found in the executable path.
                // See the proc man page for details.
                proc_path = internal::PROC_FS_PATH;  // Build the path (/proc/<pid>/stat)
                proc_path /= pid_str;
                proc_path /= "stat";
                std::ifstream proc_stat_strm(proc_path.string());
                if (proc_stat_strm.is_open())
                {
                    constexpr auto EXPECTED_STAT_TOKEN_COUNT = 44;
                    std::string line;
                    std::getline(proc_stat_strm, line);
                    std::vector<std::string> proc_stat_tokens;
                    boost::algorithm::split(
                        proc_stat_tokens, line, boost::algorithm::is_any_of(" "));
                    if (!proc_stat_tokens.empty() &&
                        proc_stat_tokens.size() >= EXPECTED_STAT_TOKEN_COUNT)
                    {
                        constexpr size_t STAT_STARTTIME_INDEX = 21;

                        // Retrieve the times and convert from ticks
                        const uint64_t hertz = sysconf(_SC_CLK_TCK);
                        starttime =
                            boost::lexical_cast<uint64_t>(proc_stat_tokens[STAT_STARTTIME_INDEX]) /
                            hertz;
                    }
                }
            }
        }
        catch (const boost::bad_lexical_cast&)
        {
            starttime = 0;
        }
        catch (const boost::filesystem::filesystem_error&)
        {
            starttime = 0;
        }
        return starttime;
    }

    bool system_process_impl::_is_numeric(const std::string& name)
    {
        if (name.empty())
        {
            return false;
        }

        // check if it is all digits
        return std::all_of(name.begin(), name.end(), ::isdigit);
    }

    boost::filesystem::path system_process_impl::_get_executable_path_from_proc_fs(pid_t pid)
    {
        boost::filesystem::path executable_path;

        try
        {
            const std::string pid_str = boost::lexical_cast<std::string>(pid);

            //
            // Get the path of the executable for this 'pid' by reading the '/proc/<pid>/exe'
            // symbolic link if that fails with permission denied it is a system process.
            // For the System process the path is found by reading '/proc/<pid>/cmdline'.
            //
            boost::filesystem::path proc_fs_path(internal::PROC_FS_PATH);  //  path to /proc files
            proc_fs_path /= pid_str;  // Build the path (/proc/<pid>/exe)
            proc_fs_path /= "exe";

            boost::system::error_code ec = {};

            executable_path = boost::filesystem::read_symlink(proc_fs_path, ec);
            if (ec.value() == boost::system::errc::permission_denied)
            {
                // Build the path '/proc/<pid>/cmdline'
                proc_fs_path = internal::PROC_FS_PATH;
                proc_fs_path /= pid_str;
                proc_fs_path /= "cmdline";

                std::ifstream proc_strm(proc_fs_path.string());
                if (proc_strm.is_open())
                {
                    std::string line;
                    std::getline(proc_strm, line);
                    // Erase all characters from the first non printable character
                    const auto pos = std::find_if_not(line.begin(),
                        line.end(),
                        [](const char& c) -> bool { return std::isprint(c); });
                    if (pos != std::end(line))
                    {
                        line.erase(pos, line.end());
                    }
                    executable_path = line;
                }
            }
        }
        catch (const boost::bad_lexical_cast&)
        {}

        return executable_path;
    }
}  // namespace bit_shovel_plugins
