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

#include "lesf/core/exception.h"

#include <utility>

using namespace lesf;
using namespace core;

Exception::Exception(BuildInfo&& build_info, const char* what) noexcept :
    std::runtime_error(what),
    m_build_info(std::move(build_info)),
    m_debug_info()
{}

Exception::Exception(BuildInfo&& build_info, std::string const& what) noexcept :
    std::runtime_error(what),
    m_build_info(std::move(build_info)),
    m_debug_info()
{}

Exception::Exception(BuildInfo&& build_info, Exception::DebugInfo&& debug_info, const char* what) noexcept :
    std::runtime_error(what),
    m_build_info(std::move(build_info)),
    m_debug_info(std::move(debug_info))
{}

Exception::Exception(BuildInfo&& build_info, Exception::DebugInfo&& debug_info, std::string const& what) noexcept :
    std::runtime_error(what),
    m_build_info(std::move(build_info)),
    m_debug_info(std::move(debug_info))
{}

Exception::Exception(const char* what) noexcept :
    std::runtime_error(what),
    m_build_info(),
    m_debug_info()
{}

Exception::Exception(std::string const& what) noexcept :
    std::runtime_error(what),
    m_build_info(),
    m_debug_info()
{}

Exception::~Exception() noexcept
{}

Exception::BuildInfo const& Exception::buildInfo() const noexcept
{
    return m_build_info;
}

Exception::DebugInfo const& Exception::debugInfo() const noexcept
{
    return m_debug_info;
}

bool Exception::recoverable() const noexcept
{
    return false;
}

bool RecoverableException::recoverable() const noexcept
{
    return true;
}
