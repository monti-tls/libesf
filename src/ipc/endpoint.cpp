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

#include "lesf/ipc/endpoint.h"
#include "lesf/ipc/exception.h"
#include "lesf/ipc/message_factory.h"

#include <atomic>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

using namespace boost::interprocess;

using namespace lesf;
using namespace ipc;

// This structure represents a one-way shared buffer between two processes.
struct Endpoint::SharedBuffer
{
public:
    SharedBuffer() :
        mutex(1),
        sem_empty(1),
        sem_full(0),
        shutdown(false)
    {}

    interprocess_semaphore mutex; // Protect access to the queue
    interprocess_semaphore sem_empty; // Semaphore to wait on when no data
    interprocess_semaphore sem_full; // Semaphore to wait on when data is here

    bool shutdown; // This flag is used to stop the receiver thread
    char buffer[Endpoint::MaxMessageSize];
};

// This is the actual data shared between client and server processes
struct Endpoint::SharedData
{
    interprocess_mutex client_mutex; // Only allow a single client per endpoint
    Endpoint::SharedBuffer buffers[2]; // The two buffers for full duplex IPC
};

// This structure is used to hold information about the shared memory between
//   the two processes. It contains two Endpoint::SharedData one-way buffers
//   to implement a full-duplex endpoint.
struct Endpoint::SharedMem
{
    shared_memory_object* shm; // Shared memory descriptor
    mapped_region* map; // Memory map to access shared memory
    Endpoint::SharedData* data; // Actual shared data structure in the map
    Endpoint::SharedBuffer* send_buf; // Pointer to the send buffer in .data (because clients have the opposite from servers)
    Endpoint::SharedBuffer* recv_buf; // Pointer to the receive buffer in .data
};

Endpoint::Endpoint(Endpoint::Role role, std::string const& name) :
    m_role(role),
    m_name(name),
    m_exc_handler(0)
{
    if (role == Server)
    {
        // Normally not required, but we do it anyway to avoid problems if the
        //   program crashed and didn't remove the resource from the system
        shared_memory_object::remove(name.c_str());

        try {
            m_shared = new SharedMem();

            // Create the shared memory region
            m_shared->shm = new shared_memory_object(create_only, name.c_str(), read_write);
            m_shared->shm->truncate(2 * sizeof(SharedData));
            m_shared->map = new mapped_region(*m_shared->shm, read_write);

            // Construct our shared memory space
            m_shared->data = new (m_shared->map->get_address()) SharedData();
            m_shared->send_buf = &m_shared->data->buffers[0];
            m_shared->recv_buf = &m_shared->data->buffers[1];

        } catch (interprocess_exception const& exc) {
            LESF_CORE_THROW(SharedMemoryException, "unable to create IPC endpoint `" << name << "` : " << exc.what());
        }
    }
    else
    {
        try {
            m_shared = new SharedMem();

            // Open the shared memory region created by the server
            m_shared->shm = new shared_memory_object(open_only, name.c_str(), read_write);
            m_shared->map = new mapped_region(*m_shared->shm, read_write);

            // Retrieve our shared memory space, initialized by the server
            m_shared->data = static_cast<SharedData*>(m_shared->map->get_address());
            m_shared->send_buf = &m_shared->data->buffers[1];
            m_shared->recv_buf = &m_shared->data->buffers[0];

            // Check if another client is already connected, if not acquire the client mutex
            /*if (!m_shared->data->client_mutex.try_lock())
            {
                delete m_shared->map;
                delete m_shared->shm;
                delete m_shared;

                LESF_CORE_THROW(SharedMemoryException, "unable to open IPC endpoint `" << name << "` : another client is already connected");
            }*/

        } catch (interprocess_exception const& exc) {
            LESF_CORE_THROW(SharedMemoryException, "unable to open IPC endpoint `" << name << "` : " << exc.what());
        }
    }

    m_receive_thread = std::thread(&Endpoint::M_receiveThread, this);
}

Endpoint::~Endpoint()
{
    // Set the shutdown flag and make sure to unblock the receiving thread
    m_shared->recv_buf->shutdown = true;
    m_shared->recv_buf->sem_full.post();
    m_receive_thread.join();
    // Don't leave this flag in case another client takes our place later on
    m_shared->recv_buf->shutdown = false;

    // Delete shared memory object if we own it
    if (m_role == Server)
    {
        /*m_shared->data->client_mutex.lock(); // wait for client to be done with the current operation, if any
        m_shared->data->client_mutex.unlock();*/
        m_shared->data->~SharedData();
    }
    // Otherwise, allow other clients to connect by releasing the client mutex
    else
    {
        // m_shared->data->client_mutex.unlock();
    }

    // Delete shared memory descriptors
    delete m_shared->map;
    delete m_shared->shm;
    delete m_shared;

    // make sure to remove the shared memory resource from the system
    if (m_role == Server)
        shared_memory_object::remove(m_name.c_str());

    if (m_exc_handler)
        delete m_exc_handler;
}

void Endpoint::send(Message const& msg)
{
    // Wait until shared data available
    m_shared->send_buf->sem_empty.wait();
    m_shared->send_buf->mutex.wait();

    // Write data to shared memory
    std::string json = MessageFactory::serialize(msg);
    if (json.size()+1 > sizeof(m_shared->send_buf->buffer))
        LESF_CORE_THROW(SharedMemoryException, "IPC packet size (" << json.size()+1 << ") exceeds limit (" << sizeof(m_shared->send_buf->buffer) << ")");
    std::strcpy(m_shared->send_buf->buffer, json.c_str());

    // std::cout << json << std::endl << std::endl;

    // Signal receiver that data is available
    m_shared->send_buf->mutex.post();
    m_shared->send_buf->sem_full.post();
}

void Endpoint::registerExceptionHandler(std::function<void(core::RecoverableException const&)> const& handler)
{
    if (m_exc_handler)
        delete m_exc_handler;

    m_exc_handler = new std::function<void(core::RecoverableException const&)>(handler);
}

void Endpoint::M_receiveThread()
{
    bool stop_thread = false;
    do
    {
        // Wait until there is some data to receive, or if the thread
        //   must terminate
        m_shared->recv_buf->sem_full.wait();
        m_shared->recv_buf->mutex.wait();

        // If asked for shutdown, terminate this thread
        if (m_shared->recv_buf->shutdown)
        {
            stop_thread = true;
        }
        else
        {
            try {
                Message* msg = 0;

                // We must respect RAII when an exception is thrown so that msg
                //   is properly deleted
                struct deleter {
                    deleter(Message** msg) : msg(msg) {}
                    ~deleter() { if (*msg) delete *msg; }
                    Message** msg;
                } _deleter(&msg);

                // Construct the message from JSON data and get the type identifier (this can throw)
                std::string type_id;
                msg = MessageFactory::construct(m_shared->recv_buf->buffer, &type_id);

                // Call the appropriate slot
                auto it = m_slots.find(type_id);
                if (it == m_slots.end())
                    LESF_CORE_THROW(DataFormatException, "IPC message type `" + type_id + "`is not connected to any slot");
                
                it->second(*this, *msg);
            } catch (core::RecoverableException const& exc) {
                if (m_exc_handler)
                    (*m_exc_handler)(exc);
            } // other exceptions will call std::terminate()
        }

        // Signal that we consumed this data
        m_shared->recv_buf->mutex.post();
        m_shared->recv_buf->sem_empty.post();
    } while (!stop_thread);
}
