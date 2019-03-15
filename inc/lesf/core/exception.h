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

#ifndef __LESF_CORE_EXCEPTION_H__
#define __LESF_CORE_EXCEPTION_H__

// This header defines a common exception class for libesf, with optional support
//   for embedded build & stack trace information.
// Sub-libraries define their own exception classes, subclassing either RecoverableException or
//   UnrecoverableException, depending on the nature of the encountered error.
// Note: Always use LESF_CORE_THROW() as it allows :
//   - automatic embedding of build & debug data
//   - easy generation of description messages based on an std::iostream style
//     format expression with exception-safe semantics

#include <exception>
#include <string>
#include <sstream>

#include <boost/stacktrace/stacktrace.hpp>

#define LESF_CORE_THROW(exc_type, msg) \
    try { \
        throw exc_type(lesf::core::Exception::BuildInfo{true, __FILE__, __LINE__, __FUNCTION__, \
                                                        __DATE__, __TIME__, LESF_USER_BUILD_ID, LESF_USER_PROGRAM, LESF_USER_VERSION}, \
                          lesf::core::Exception::DebugInfo{true, boost::stacktrace::stacktrace()}, \
                          lesf::core::Exception::InfoStringBuilder() << msg); \
    } catch(lesf::core::Exception const& exc) { \
        throw; \
    } catch(std::exception const& exc) { \
        throw exc_type(lesf::core::Exception::BuildInfo{true, __FILE__, __LINE__, __FUNCTION__, \
                                                        __DATE__, __TIME__, LESF_USER_BUILD_ID, LESF_USER_PROGRAM, LESF_USER_VERSION}, \
                          lesf::core::Exception::DebugInfo{true, boost::stacktrace::stacktrace()}, \
                          "<unable to construct description string> `" #msg "'"); \
    }

namespace lesf { namespace core {

class Exception : public std::runtime_error
{
    friend class RecoverableException;
    friend class UnrecoverableException;

public:
    struct BuildInfo
    {
        bool set;
        const char* file;
        int line;
        const char* function;
        const char* build_date;
        const char* build_time;
        const char* user_build_id;
        const char* user_program;
        const char* user_version;
    };

    struct DebugInfo
    {
        bool set;
        boost::stacktrace::stacktrace trace;
    };

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
    explicit Exception(BuildInfo&& build_info, const char* what) noexcept;
    explicit Exception(BuildInfo&& build_info, std::string const& what) noexcept;

    explicit Exception(BuildInfo&& build_info, DebugInfo&& debug_info, const char* what) noexcept;
    explicit Exception(BuildInfo&& build_info, DebugInfo&& debug_info, std::string const& what) noexcept;

    explicit Exception(const char* what) noexcept;
    explicit Exception(std::string const& what) noexcept;

    virtual ~Exception() noexcept;

    BuildInfo const& buildInfo() const noexcept;
    DebugInfo const& debugInfo() const noexcept;

    virtual bool recoverable() const noexcept;

private:
    BuildInfo m_build_info;
    DebugInfo m_debug_info;
};

class RecoverableException : public Exception
{
    using Exception::Exception;

public:
    bool recoverable() const noexcept;
};

class UnrecoverableException : public Exception
{
    using Exception::Exception;
};

} }

#endif // __LESF_CORE_EXCEPTION_H__
