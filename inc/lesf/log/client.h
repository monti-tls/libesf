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

#ifndef __LESF_LOG_CLIENT_H__
#define __LESF_LOG_CLIENT_H__

#include "lesf/log/config.h"

#include <string>
#include <deque>
#include <array>

namespace lesf { namespace log {
// This class is used to connect to a LogServer at a given server & address
//   in order to send logs.
// LogClient::write() can be called even if the client is not yet connected to the
//   server. Data will be queued until connection is gained. In case of disconnection or
//   socket error, LogClient will attempt to regain the connection indefinitely.

class Client
{
public:
    enum
    {
        ConnectionRetryTimeout = 10 // ms
    };

private:
    class Internals; // Used to hide boost::asio stuff from this header

public:
    Client(unsigned short port = Config::ServerPort, std::string const& server = "localhost");
    ~Client();

    Client(Client const&) = delete;
    Client& operator=(Client const&) = delete;

    void send(std::string const& msg);

private:
    void M_ioThread(); // Thread for boost::io_context.
    void M_doConnect(); // Attempt to connect to remote server.
    void M_doRead(); // Monitor socket status, when connected.
    void M_queueMessage(std::string msg_with_header); // Schedule message for sending.
    void M_doWrite(); // Attempt to write queued message

private:
    unsigned short m_port;
    std::string m_server;
    Internals* m_internals;
    bool m_connected;
    bool m_terminate;
    std::deque<std::string> m_message_queue;
    std::array<char, 1> m_dummy; // Dummy buffer used for M_doRead()
};

} }

#endif // __LESF_LOG_CLIENT_H__
