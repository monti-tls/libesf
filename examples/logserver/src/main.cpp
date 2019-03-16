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

#include <iostream>

#include "lesf/lesf.h"

using namespace lesf;

LESF_CONFIG_SYMBOLS()

namespace outputs {

    template <typename formatterT>
    class Ostream : public log::Server::Subscriber
    {
    public:
        Ostream(log::Server& server, std::ostream& os) :
            log::Server::Subscriber(server),
            m_fmt(os)
        {}

    private:
        void M_notifyMessageReceived(log::Message const& msg)
        {
            m_fmt(msg);
        }

    private:
        formatterT m_fmt;
    };

    template <typename formatterT>
    class String : public log::Server::Subscriber
    {
    public:
        String(log::Server& server) :
            log::Server::Subscriber(server),
            m_ss(),
            m_fmt(m_ss)
        {}

        operator std::string() const
        {
            return m_ss.str();
        }

    private:
        void M_notifyMessageReceived(log::Message const& msg)
        {
            m_fmt(msg);
        }

    private:
        std::ostringstream m_ss;
        formatterT m_fmt;
    };

}

class LogService : public daemon::Service
{
public:
    LogService() :
        m_server(new log::Server()),
        m_out(*m_server, std::cout)
    {
    }

    ~LogService()
    {}

    void run() noexcept
    {
        while (m_server);
    }

    void restart()
    {

    }

    void stop()
    {
        delete m_server;
        m_server = 0;
    }

private:
    log::Server* m_server;
    outputs::Ostream<log::formatters::plain_light> m_out;
};

int main(int argc, char** argv)
{
    daemon::daemonize([]() { return new LogService(); });
}
