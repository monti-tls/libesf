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

#include "lesf/log/server.h"
#include "lesf/log/session.h"
#include "lesf/log/session_internals.h"

#include <memory>
#include <thread>
#include <set>
#include <iostream>

#include <boost/asio.hpp>

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

using namespace lesf;
using namespace log;

Server::Subscriber::Subscriber(Server& server) :
    m_server(&server)
{
    m_server->m_subscribers.insert(this);
}

Server::Subscriber::~Subscriber()
{
    if (m_server)
        m_server->m_subscribers.erase(this);
}

void Server::Subscriber::M_notifyServerDeleted()
{}

class Server::Internals
{
public:
    Internals(unsigned short port) :
        ctx(),
        endpoint(tcp::v4(), port),
        acceptor(ctx, endpoint)
    {}

    io_context ctx;
    tcp::endpoint endpoint;
    tcp::acceptor acceptor;
    std::thread io_thread;
    std::set<std::shared_ptr<Session>> sessions;
};

Server::Server(unsigned short port) :
    m_internals(new Server::Internals(port)),
    m_subscribers{}
{
    m_internals->io_thread = std::thread(&Server::M_ioThread, this);
    M_doAccept();
}

Server::~Server()
{
    for (auto it : m_subscribers)
    {
        it->m_server = 0;
        it->M_notifyServerDeleted();
    }

    m_internals->ctx.post([this]() { m_internals->ctx.stop(); });
    m_internals->io_thread.join();
    delete m_internals;
}

void Server::M_ioThread()
{
    m_internals->ctx.run();
}

void Server::M_doAccept()
{
    m_internals->acceptor.async_accept(
        [this](error_code const& err, tcp::socket&& socket)
        {
            if (!err)
            {
                std::make_shared<Session>(Session::Internals(std::move(socket), m_internals->sessions, this))->start();

                M_doAccept();
            }
        });
}

void Server::M_notifyMessageReceived(std::string const& data)
{
    Message msg = Message::synthetize(data);

    for (auto it : m_subscribers)
        it->M_notifyMessageReceived(msg);
}
