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

#ifndef __LESF_LOG_CONFIG_H__
#define __LESF_LOG_CONFIG_H__

namespace lesf { namespace log {

namespace Config
{
    enum : unsigned short
    {
        ServerPort = 50001U,
    };

    extern const char* FallbackLogFilePrefix;
};

#define LESF_LOG_CONFIG_SYMBOLS() \
    namespace lesf { namespace log { namespace Config { \
        const char* FallbackLogFilePrefix = "/var/log/" LESF_USER_PROGRAM ".fallback_log."; \
    } } }

} }

#endif // __LESF_LOG_CONFIG_H__
