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

#include "lesf/log/session.h"
#include "lesf/log/session_internals.h"
#include "lesf/log/server.h"

#include <set>
#include <string>
#include <utility>

#include <boost/asio.hpp>

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

using namespace lesf;
using namespace log;

Session::Internals::Internals(Session::Internals&& cpy) :
        socket(std::move(cpy.socket)),
        sessions(cpy.sessions),
        server(cpy.server)
{}

Session::Internals::Internals(tcp::socket&& socket, std::set<std::shared_ptr<Session>>& sessions, Server* server) :
    socket(std::move(socket)),
    sessions(sessions),
    server(server)
{}

Session::Session(Session::Internals&& internals) :
    m_internals(new Session::Internals(std::move(internals)))
{
}

Session::~Session()
{
    /*FIXME: gosh, is it really okay ? or even useful ? hard to test

    // If there's some data to read still, wait until it's done before dying
    // It's okay to block in this destrutor as read is performed in another thread
    //   (namely llog::Server::M_ioThread()).
    // We use the available(error_code&) version because the other one throws if an error occur,
    //   which we just want to ignore here
    error_code err;
    while (m_internals->socket.available(err));

    */

    delete m_internals;
}

void Session::start()
{
    m_internals->sessions.insert(shared_from_this());
    M_doReadHeader();
}

void Session::M_doReadHeader()
{
    auto self(shared_from_this());
    async_read(m_internals->socket, buffer(m_header),
        [this, self](error_code const& err, std::size_t)
        {
            if (!err)
            {
                std::size_t size = std::atoi(std::string(m_header.begin(), m_header.end()).c_str());
                m_message.resize(size);
                M_doReadMessage();
            }
            else
            {
                m_internals->sessions.erase(shared_from_this());
            }
        });
}

void Session::M_doReadMessage()
{
    auto self(shared_from_this());
    async_read(m_internals->socket, buffer(m_message),
        [this, self](error_code const& err, std::size_t)
        {
            if (!err)
            {
                m_internals->server->M_notifyMessageReceived(std::string(m_message.begin(), m_message.end()));
                M_doReadHeader();
            }
            else
            {
                m_internals->sessions.erase(shared_from_this());
            }
        });
}
