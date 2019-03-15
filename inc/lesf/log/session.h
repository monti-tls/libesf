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

#ifndef __LESF_LOG_SESSION_H__
#define __LESF_LOG_SESSION_H__

#include <memory>
#include <vector>
#include <array>

namespace lesf { namespace log {

// This class represents a connection from a client on the server side.
// Memory management is a bit complicated due to asynchronous handlers so
//   we use std::shared_ptr<> and shared_from_this() to keep the objects alive
//   until all handlers completed execution.
// In order to hide boost::asio stuff from the headers, we use the Session::Internals
//   class (see llog/session_internals.h)
// This is not a user class.
class Session : public std::enable_shared_from_this<Session>
{
    friend class Server; // The log::Server class needs to access Session::Internals

protected:
    class Internals;  // Used to hide boost::asio stuff from this header

public:
    enum
    {
        HeaderSize = 8UL
    };

public:
    Session(Internals&& internals);
    ~Session();

    Session(Session const&) = delete;
    Session& operator=(Session const&) = delete;

    void start();

private:
    void M_doReadHeader(); // Wait until message header (size) is received
    void M_doReadMessage(); // Wait until message data is received

private:
    Internals* m_internals;
    std::array<char, HeaderSize> m_header;
    std::vector<char> m_message;
};

} }

#endif // __LESF_LOG_SESSION_H__
