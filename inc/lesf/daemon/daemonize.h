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

#ifndef __LESF_DAEMON_DAEMONIZE_H__
#define __LESF_DAEMON_DAEMONIZE_H__

#include <functional>

#include "lesf/daemon/service.h"

namespace lesf { namespace daemon {

void daemonize(std::function<Service*()> const& ctor) __attribute__((noreturn));

} }

#endif // __LESF_DAEMON_DAEMONIZE_H__
