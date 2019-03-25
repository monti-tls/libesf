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

#ifndef __LESF_LOG_LOGGER_H__
#define __LESF_LOG_LOGGER_H__

#include "lesf/core/exception.h"
#include "lesf/log/config.h"
#include "lesf/log/message.h"
#include "lesf/log/client.h"

#include <string>
#include <sstream>
#include <fstream>
#include <mutex>

#define LESF_LOG_TRACE(fmt)   LESF_LOG(Trace, fmt)
#define LESF_LOG_INFO(fmt)    LESF_LOG(Info, fmt)
#define LESF_LOG_WARNING(fmt) LESF_LOG(Warning, fmt)
#define LESF_LOG_ERROR(fmt)   LESF_LOG(Error, fmt)
#define LESF_LOG_CRASH(fmt)   LESF_LOG(Crash, fmt)

#define LESF_LOG_TRACE_WITH_EXCEPT(exc, fmt)   LESF_LOG_WITH_EXCEPT(Trace, exc, fmt)
#define LESF_LOG_INFO_WITH_EXCEPT(exc, fmt)    LESF_LOG_WITH_EXCEPT(Info, exc, fmt)
#define LESF_LOG_WARNING_WITH_EXCEPT(exc, fmt) LESF_LOG_WITH_EXCEPT(Warning, exc, fmt)
#define LESF_LOG_ERROR_WITH_EXCEPT(exc, fmt)   LESF_LOG_WITH_EXCEPT(Error, exc, fmt)
#define LESF_LOG_CRASH_WITH_EXCEPT(exc, fmt)   LESF_LOG_WITH_EXCEPT(Crash, exc, fmt)

#define LESF_LOG(type, fmt) \
    try { \
        lesf::log::Logger::log(lesf::log::Message( \
            lesf::log::Message::type, \
            { __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__, \
              LESF_USER_BUILD_ID, LESF_USER_PROGRAM, LESF_USER_VERSION }, \
            lesf::log::Logger::getProcessInfo(), \
            (lesf::log::Logger::InfoStringBuilder() << fmt))); \
    } catch (std::exception const& e) { \
        lesf::log::Logger::log(e, #type, #fmt); \
    }

#define LESF_LOG_WITH_EXCEPT(type, exc, fmt) \
    try { \
        lesf::log::Logger::log(lesf::log::Message( \
            lesf::log::Message::type, \
            { __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__, \
              LESF_USER_BUILD_ID, LESF_USER_PROGRAM, LESF_USER_VERSION }, \
            lesf::log::Logger::getProcessInfo(), \
            (lesf::log::Logger::InfoStringBuilder() << fmt), \
            (exc))); \
    } catch (std::exception const& e) { \
        lesf::log::Logger::log(e, #type, #fmt); \
    }

namespace lesf { namespace log {

class Logger
{
public:
    class AutoDeleter;

public:
    class InfoStringBuilder
    {
    public:
        template <typename T>
        InfoStringBuilder& operator<<(T const& t)
        {
            m_os << t;
            return *this;
        }

        operator std::string() const
        {
            return m_os.str();
        }

    private:
        std::ostringstream m_os;
    };

public:
    // Initialize the logger
    // This function must be called at the beginning of the program, otherwise
    //   the logger will be initialized at the first log request but without
    //   (argc, argv) information and with the default port.
    //
    // This function is specified as noexcept and handles exceptions when creating
    //  the internal log::Client for remote logging, but if we fail to create
    //  the logger instance log::Logger::m_inst, the program will terminate.
    static void init(int argc, char** argv, unsigned short port = Config::ServerPort,
                     const char* fallback_log_file_prefix = Config::FallbackLogFilePrefix) noexcept;

    // Log a message. If the remote client could be created, send it over here.
    // If there's any problem, try to log the message in the fallback file, otherwise
    //   terminate the program.
    static void log(Message const& msg) noexcept;

    // (fallback solution when an exception is encountered in LESF_LOG_*)
    // Attempt to write information to the fallback file, terminate the program
    //   if we can't.
    static void log(std::exception const& exc, const char* msg_type, const char* msg_fmt) noexcept;

    static Message::ProcessInfo getProcessInfo();

private:
    Logger(int argc, char** argv, unsigned short port, const char* fallback_log_file_prefix);
    ~Logger();

    static void M_maybeInstanciate();

private:
    static std::mutex m_mutex;
    static Logger* m_inst;

    unsigned short m_remote_port;
    int m_argc;
    char** m_argv;
    int m_pid;
    Client* m_remote;

    std::string m_fallback_fn;
    std::ofstream m_fallback;
};

} }

#endif // __LESF_LOG_LOGGER_H__
