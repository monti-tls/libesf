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

#include "lesf/log/formatter.h"

#include <iomanip>
#include <ctime>
#include <cxxabi.h>

#include <boost/algorithm/hex.hpp>

using namespace lesf;
using namespace log;
using namespace formatters;

Formatter::Formatter(std::ostream& os) :
    m_os(os)
{}

Formatter::~Formatter()
{}

std::ostream& Formatter::M_os()
{
    return m_os;
}

void raw::operator()(Message const& msg)
{
    M_os() << Message::serialize(msg) << std::endl;
}

void plain_light::operator()(Message const& msg)
{
    static std::map<log::Message::Type, const char*> type_str_map =
    {
        { log::Message::Unknown, "Unknown:" },
        { log::Message::Crash,   "Crash:  " },
        { log::Message::Error,   "Error:  " },
        { log::Message::Warning, "Warning:" },
        { log::Message::Info,    "Info:   " },
        { log::Message::Trace,   "Trace:  " }
    };

    // Time
    std::time_t time = std::chrono::system_clock::to_time_t(msg.time());
    std::string time_str = std::ctime(&time);
    time_str.erase(time_str.end()-1);
    M_os() << time_str << " | ";

    // Software
    M_os() << std::setw(8) << std::left << msg.buildInfo().user_program << " <";
    M_os() << msg.buildInfo().user_build_id << "> | ";

    // Process information
    M_os() << std::setw(8) << std::left << msg.processInfo().process << " (";
    M_os() << std::setw(5) << std::right << std::setfill(' ') << msg.processInfo().pid << ") | ";

    // Log type and message
    auto type_str = type_str_map.find(msg.type());
    M_os() << (type_str != type_str_map.end() ? type_str->second : "???:    ");
    M_os() << msg.message();

    if (msg.exceptionInfo())
    {
        log::Message::ExceptionInfo const& exc = *msg.exceptionInfo();

        M_os() << " (exception details follows)" << std::endl;

        std::string const indent = "    ";
        
        // Software
        M_os() << indent << "in " << exc.build_info.user_program << " <";
        M_os() << exc.build_info.user_build_id << ">" << std::endl;

        // Build info
        M_os() << indent << "at " << exc.build_info.file << ":" << exc.build_info.line << " (in " << exc.build_info.function << ")" << std::endl;

        // Exception details
        char* exc_type = __cxxabiv1::__cxa_demangle(exc.rtti_type.c_str(), 0, 0, 0);
        M_os() << indent << "exception '" << (exc_type ? exc_type : exc.rtti_type) << "': " << exc.what << std::endl;
        if (exc_type)
            free(exc_type);

        // Stack trace
        if (!exc.trace.empty())
        {
            M_os() << indent << "stack trace:" << std::endl;

            std::string trace;
            boost::algorithm::unhex(exc.trace.begin(), exc.trace.end(), std::back_inserter(trace));

            std::size_t p = 0;
            do
            {
                p = trace.find("\n", p+1);
                if (p == std::string::npos || p == trace.size()-1)
                    break;

                trace.replace(p, 1, std::string("\n") + indent + indent);
            } while (p != std::string::npos);

            M_os() << indent << indent << std::string(trace.begin(), trace.end()-1);
        }
        else
            M_os() << indent << "no stack trace available";
    }

    M_os() << std::endl;
}
