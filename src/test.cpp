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
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <ctime>
#include <cxxabi.h>

#include <boost/algorithm/hex.hpp>

#include "lesf/core/core.h"
#include "lesf/ipc/ipc.h"
#include "lesf/log/log.h"

using namespace lesf;

namespace outputs {

    template <typename formatterT>
    class Stdout : public log::Server::Subscriber
    {
    public:
        Stdout(log::Server& server) :
            log::Server::Subscriber(server),
            m_fmt(std::cout)
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

int main(int argc, char** argv)
{
    log::Server srv;
    outputs::Stdout<log::formatters::plain_light> out_plain(srv);
    outputs::String<log::formatters::raw> out_raw(srv);

    try {
        log::Logger::init(argc, argv);

        LESF_LOG_TRACE("yolo " << 5976);

        try {
            LESF_CORE_THROW(ipc::SharedMemoryException, "prout");
        } catch (core::Exception const& exc) {
            LESF_LOG_TRACE_WITH_EXCEPT(exc, "exception caught");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } catch (core::Exception const& exc) {
        std::cerr << exc.what() << std::endl;
        std::cerr << exc.debugInfo().trace << std::endl;
    } catch (std::exception const& exc) {
        std::cerr << "oulala" << std::endl;
    }

    std::cerr << (std::string) out_raw;

    return 0;
}
