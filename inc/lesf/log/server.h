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

#ifndef __LESF_LOG_SERVER_H__
#define __LESF_LOG_SERVER_H__

#include <string>
#include <set>

#include "lesf/log/config.h"
#include "lesf/log/message.h"

namespace lesf { namespace log {

// A log server, listening for messages on a TCP port. Multiple
//   clients can connect/disconnect at any time, messages
//   are not bound to any particular client.
class Server
{
    friend class Session; // log::Session needs to access M_notifyMessageReceived()

public:
    class Subscriber
    {
        friend class Server;
        
    public:
        Subscriber(Server& server);
        virtual ~Subscriber();

    protected:
        virtual void M_notifyServerDeleted();
        virtual void M_notifyMessageReceived(Message const& msg) = 0;

    private:
        Server* m_server;
    };

private:
    class Internals; // Used to hide boost::asio stuff from this header

public:
    Server(unsigned short port = Config::ServerPort);
    ~Server();

    Server(Server const&) = delete;
    Server& operator=(Server const&) = delete;

private:
    void M_ioThread();
    void M_doAccept();

    void M_notifyMessageReceived(std::string const& data);

private:
    Internals* m_internals;
    std::set<Subscriber*> m_subscribers;
};

} }

#endif // __LESF_LOG_SERVER_H__
