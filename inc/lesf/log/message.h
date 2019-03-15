/* This file is part of libesf.
 * 
 * Copyright (c) 2019, Alexandre Monti
 * 
 * libesf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * libesf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with libesf.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LESF_LOG_MESSAGE_H__
#define __LESF_LOG_MESSAGE_H__

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <chrono>

#include "lconf/json.h"

#include "lesf/core/exception.h"

namespace lesf { namespace log {

using namespace lconf;

class Message
{
public:
    enum Type
    {
        Unknown = -2, // This corresponds to a failed serialization of the log message
        Crash = -1,
        Error = 0,
        Warning = 1,
        Info = 2,
        Trace = 3
    };

    struct BuildInfo
    {
        std::string file;
        int line;
        std::string function;
        std::string build_date;
        std::string build_time;
        std::string user_build_id;
        std::string user_program;
        std::string user_version;
    };
    
    struct ProcessInfo
    {
        std::string process;
        int pid;
    };

    struct ExceptionInfo
    {
        struct BuildInfo
        {
            std::string file;
            int line;
            std::string function;
            std::string build_date;
            std::string build_time;
            std::string user_build_id;
            std::string user_program;
            std::string user_version;
        };

        std::string what;
        std::string rtti_type;
        BuildInfo build_info;
        std::string trace;
    };

    typedef std::shared_ptr<ExceptionInfo> ExceptionInfoPtr;

public:
    Message(Type type, BuildInfo&& build_info, ProcessInfo&& process_info, 
            std::string const& message);
    Message(Type type, BuildInfo&& build_info, ProcessInfo&& process_info,
            std::string const& message, core::Exception const& exception);
    ~Message();

    Type type() const;
    
    BuildInfo const& buildInfo() const;
    ProcessInfo const& processInfo() const;
    std::string const& message() const;
    std::chrono::system_clock::time_point const& time() const;
    ExceptionInfoPtr exceptionInfo() const;

    static std::string serialize(Message const& msg) noexcept;
    static Message synthetize(std::string const& serialized) noexcept;

private:
    json::Template M_jsonTemplate();
    json::Template M_buildInfoJsonTemplate();
    json::Template M_processInfoJsonTemplate();
    json::Template M_exceptionInfoJsonTemplate();

private:
    Type m_type;
    BuildInfo m_build_info;
    ProcessInfo m_process_info;
    std::chrono::system_clock::time_point m_time;
    std::string m_message;
    ExceptionInfoPtr m_exception_info;
};

} }

#endif // __LESF_LOG_MESSAGE_H__
