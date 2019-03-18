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

#ifndef __LESF_DAEMON_CONFIG_H__
#define __LESF_DAEMON_CONFIG_H__

namespace lesf { namespace daemon {

namespace Config
{
    extern const char* LogFile;
    extern const char* LockFile;
};

#define LESF_DAEMON_CONFIG_SYMBOLS() \
    namespace lesf { namespace daemon { namespace Config { \
        const char* LogFile = "/var/log/" LESF_USER_PROGRAM ".daemon_log"; \
        const char* LockFile = "/var/run/" LESF_USER_PROGRAM; \
    } } }

} }

#endif // __LESF_DAEMON_CONFIG_H__
