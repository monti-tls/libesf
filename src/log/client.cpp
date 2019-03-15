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

#include "lesf/log/client.h"
#include "lesf/log/session.h"

#include <thread>
#include <sstream>
#include <chrono>

#include <boost/asio.hpp>

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

using namespace lesf;
using namespace log;

class Client::Internals
{
public:
    Internals() :
        ctx(),
        socket(ctx)
    {}

public:
    io_context ctx;
    tcp::socket socket;
    std::thread io_thread;
};

Client::Client(unsigned short port, std::string const& server) :
    m_port(port),
    m_server(server),
    m_internals(new Client::Internals()),
    m_connected(false),
    m_terminate(false)
{
    M_doConnect();
    m_internals->io_thread = std::thread(&Client::M_ioThread, this);
}

Client::~Client()
{
    // Wait until all messages have been sent (if connected)
    while (m_connected && !m_message_queue.empty());

    // Stop I/O thread
    m_terminate = true;
    post(m_internals->ctx, [this]() { m_internals->socket.close(); });
    m_internals->io_thread.join();
    delete m_internals;
}

void Client::send(std::string const& msg)
{
    // Add the header to the message
    char header[Session::HeaderSize+1];
    sprintf(header, "%0*lu", Session::HeaderSize, msg.size());
    std::string msg_with_header = std::string(header) + msg;

    // Schedule the message for sending
    M_queueMessage(msg_with_header);
}

// This thread runs the boost::asio::io_context object's event loop.
// Restarts are needed because we don't read anything from the socket,
//   so io_context::run() will return quite often.
void Client::M_ioThread()
{
    do
    {
        m_internals->ctx.run();
        m_internals->ctx.restart();
    } while (!m_terminate);
}

// Attempt to open a connection with the server. If the async connect fails,
//   try again after LogClient::ConnectionRetryTimeout milliseconds.
void Client::M_doConnect()
{
    // Avoid an infinite event loop when we try to destroy the LogClient and
    //   it fails to connect.
    if (m_terminate)
        return;

    // Resolve TCP endpoints associated to the (server, port) target
    tcp::resolver resolver(m_internals->ctx);
    std::ostringstream ss;
    ss << m_port;
    std::string port_str = ss.str();
    auto endpoints = resolver.resolve(m_server, port_str);

    // Schedule an asynchronous connection attempt
    async_connect(m_internals->socket, endpoints,
        [this](error_code const& err, tcp::endpoint const&)
        {
            if (!err)
            {
                // Avoid delayed TCP_ACK
                m_internals->socket.set_option(tcp::no_delay(true));

                m_connected = true;
                M_doRead(); // To monitor socket status

                // Send any queued messages
                if (!m_message_queue.empty())
                    M_doWrite();
            }
            else
            {
                m_connected = false;

                // Attempt to connect again after a small timeout
                std::this_thread::sleep_for(std::chrono::milliseconds(ConnectionRetryTimeout));
                M_doConnect();
            }
        });
}

// We use an async read just to monitor socket status, otherwise if the connection is closed
//   by the peer, the first write will not return any error code and we lose a message.
void Client::M_doRead()
{
    async_read(m_internals->socket, buffer(m_dummy),
        [this](error_code const& err, std::size_t)
        {
            if (!err)
            {
                // If we read something, monitor again
                if (!m_terminate)
                    M_doRead();
            }
            else
            {                
                // If we've been disconnected, start the connection attempt event loop again
                m_connected = false;
                M_doConnect();
            }
        });
}

void Client::M_queueMessage(std::string message_with_header)
{
    post(m_internals->ctx,
        [this, message_with_header]()
        {
            // Put the message in the outgoing queue
            bool write_in_progress = !m_message_queue.empty();
            m_message_queue.push_back(message_with_header);

            // If we're conected and not already in a M_doWrite event loop,
            //   try to send the message
            if (m_connected && !write_in_progress)
                M_doWrite();
        });
}

void Client::M_doWrite()
{
    async_write(m_internals->socket, buffer(m_message_queue.front()),
        [this](error_code const& err, std::size_t)
        {
            if (!err)
            {
                // Pop the message from the outgoing queue
                m_message_queue.pop_front();

                // If there's still some messages to send, try to do so
                if (!m_message_queue.empty())
                    M_doWrite();
            }
            else
            {
                // If we encoutered a problem, close our socket and restart the
                //   connection event loop
                m_connected = false;
                m_internals->socket.close();
                M_doConnect();
            }
        });
}
