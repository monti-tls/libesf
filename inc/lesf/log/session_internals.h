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

#ifndef __LESF_LOG_SESSION_INTERNALS_H__
#define __LESF_LOG_SESSION_INTERNALS_H__

#include <set>
#include <memory>
#include <boost/asio.hpp>

namespace lesf { namespace log {

using boost::asio::ip::tcp;

class Server;

class Session::Internals
{
public:
    Internals(Internals&& cpy);
    Internals(tcp::socket&& socket, std::set<std::shared_ptr<Session>>& sessions, Server* server);

    tcp::socket socket;
    std::set<std::shared_ptr<Session>>& sessions;
    Server* server;
};

} }

#endif // __LESF_LOG_SESSION_INTERNALS_H__
