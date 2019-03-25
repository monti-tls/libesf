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

#include "lesf/log/logger.h"
#include "lesf/log/client.h"

#include <cstdlib>
#include <fstream>
#include <cstdio>

using namespace lesf;
using namespace log;

class Logger::AutoDeleter
{
public:
    ~AutoDeleter()
    {
        if (Logger::m_inst)
            delete Logger::m_inst;
    }
};

Logger* Logger::m_inst = 0;
std::mutex Logger::m_mutex;


static Logger::AutoDeleter m_inst_autodelete;

void Logger::init(int argc, char** argv, unsigned short port, const char* fallback_log_file_prefix) noexcept
{
    std::lock_guard<std::mutex> lock(Logger::m_mutex);

    m_inst = new Logger(argc, argv, port, fallback_log_file_prefix);

    try {
        m_inst->m_remote = new Client(port);
    } catch (std::exception const& exc) {
        m_inst->m_fallback << "Failed to create remote client due to active exception ("
                           << typeid(exc).name() << "): " << exc.what() << std::endl
                           << "Falling back to raw file logging." << std::endl << std::endl;
        m_inst->m_fallback.flush();

        m_inst->m_remote = 0;
    }
}

void Logger::log(Message const& msg) noexcept
{
    std::lock_guard<std::mutex> lock(Logger::m_mutex);

    M_maybeInstanciate();

    if (m_inst->m_remote)
    {
        try {
            m_inst->m_remote->send(Message::serialize(msg));
        } catch (std::exception const& exc) {
            m_inst->m_fallback << "Failed to send log message due to active exception ("
                               << typeid(exc).name() << ") : " << exc.what() << "." << std::endl
                               << "Serialized message follows :" << std::endl
                               << Message::serialize(msg) << std::endl << std::endl;
            m_inst->m_fallback.flush();
        }
    }
    else
    {
        m_inst->m_fallback << "Failed to send log message (no remote client)."
                           << std::endl << "Serialized message follows :" << std::endl
                           << Message::serialize(msg) << std::endl << std::endl;
        m_inst->m_fallback.flush();
    }
}

void Logger::log(std::exception const& exc, const char* msg_type, const char* msg_fmt) noexcept
{
    std::lock_guard<std::mutex> lock(Logger::m_mutex);

    M_maybeInstanciate();

    //TODO: maybe try to send the information over to the remote server ?
    //      I fear if I try to do a LESF_LOG_* here we may start an infinite
    //      exception loop, which would destroy the stack and ultimately end the world.

    m_inst->m_fallback << "Failed to construct log message due to active exception ("
                       << typeid(exc).name() << ") : " << exc.what() << "." << std::endl
                       << "msg_type: " << msg_type << std::endl
                       << "msg_fmt:  " << msg_fmt << std::endl << std::endl;
    m_inst->m_fallback.flush();
}

Message::ProcessInfo Logger::getProcessInfo()
{
    if (m_inst && m_inst->m_argv)
        return Message::ProcessInfo{m_inst->m_argc ? m_inst->m_argv[0] : "<no argv[0]>", m_inst->m_pid};

    return Message::ProcessInfo();
}

Logger::Logger(int argc, char** argv, unsigned short port, const char* fallback_log_file_prefix) :
    m_remote_port(port),
    m_argc(argc),
    m_argv(argv),
    m_pid(getpid()),
    m_remote(0)
{
    std::ostringstream fn;
    fn << fallback_log_file_prefix << m_pid;
    m_fallback_fn = fn.str();

    m_fallback.open(m_fallback_fn, std::ios::out | std::ios::app);
}

Logger::~Logger()
{
    // Remove the fallback log file if we haven't written anything into it
    long l, m;
    std::ifstream fallback(m_fallback_fn, std::ios::in | std::ios::binary); 
    l = fallback.tellg();
    fallback.seekg (0, std::ios::end); 
    m = fallback.tellg();
    fallback.close();
    if (!(m-l))
        remove(m_fallback_fn.c_str());

    // Destroy our client for remote logging
    if (m_remote)
        delete m_remote;
}

void Logger::M_maybeInstanciate()
{
    bool okay = m_inst;
    if (!okay)
    {
        m_inst = new Logger(0, 0, Config::ServerPort, Config::FallbackLogFilePrefix);

        LESF_LOG_WARNING("lesf::Logger::log() called before lesf::log::Logger::init(), "
                         "starting log client with default parameters and no process information.");
    }
}
