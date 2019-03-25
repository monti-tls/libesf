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

#ifndef __LESF_IPC_ENDPOINT_H__
#define __LESF_IPC_ENDPOINT_H__

#include "lesf/ipc/message.h"
#include "lesf/ipc/message_factory.h"

#include <string>
#include <map>
#include <thread>
#include <functional>

namespace lesf { namespace ipc {

// This class provides an easy to use named IPC endpoint. Endpoints are system-wide
//   resources, so be sure to use a unique name.
// Only one client can be connected to a server endpoint at a time.
class Endpoint
{
public:
    enum Role
    {
        Server,
        Client
    };

    // Maximum allowed message size.
    static const size_t MaxMessageSize = 4096UL;

public:
    // Create a new named IPC endpoint.
    // If role == ipc::Endpoint::Server, throws if name is already used
    // If role == ipc::Endpoint::Client, throws if another client is already connected
    Endpoint(Role role, std::string const& name);

    // Careful! The destructor of a Server endpoint will block until the client
    //   is detroyed.
    ~Endpoint();

    // Send a message over the endpoint
    void send(Message const& msg);

    // Register a handler for a particular message type. The given handler will
    //   be called from another thread when a message of this type is received.
    // Any exception raised from this thread is catched and :
    //   - Discarded if no exception handler is registered
    //   - Passed as an argument to the registered exception handler
    template <typename T>
    void registerSlot(std::function<void(Endpoint&, T const&)> const& handler)
    {
        auto id = MessageFactory::typeIdentifier<T>();
        m_slots[id] = [handler](Endpoint& ep, Message const& msg) { handler(ep, dynamic_cast<T const&>(msg)); };
    }

    // Register an exception handler for the receiving thread.
    void registerExceptionHandler(std::function<void(core::RecoverableException const&)> const& handler);

private:
    // This method runs in another thread and wait for anything to be received
    void M_receiveThread();

private:
    // Some internal data types (see endpoint.cpp for details) to manage shared memory
    struct SharedBuffer;
    struct SharedData;
    struct SharedMem;

private:
    Role m_role;
    std::string m_name;

    SharedMem* m_shared;
    std::thread m_receive_thread;
    std::map<std::string, std::function<void(Endpoint&, Message const&)>> m_slots;
    std::function<void(core::RecoverableException const&)>* m_exc_handler;
};

} }

#endif // __LESF_IPC_ENDPOINT_H__
