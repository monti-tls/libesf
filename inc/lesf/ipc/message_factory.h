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

#ifndef __LESF_IPC_MESSAGE_FACTORY__
#define __LESF_IPC_MESSAGE_FACTORY__

#include <string>
#include <map>
#include <functional>
#include <type_traits>

#include "lesf/ipc/exception.h"
#include "lesf/ipc/message.h"

namespace lesf { namespace ipc {

// This class exposes static functions to register ipc::Message-based concrete
//   types into the system.
// Each concrete type is associated with a string identifier. This class then
//   provides static functions to :
//     - construct an ipc::Message* from JSON
//     - serialize an ipc::Message* into JSON
class MessageFactory
{
private:
    MessageFactory() {}
    ~MessageFactory() {}

public:
    // Register a new concrete message type in the system using the given identifier.
    template <typename T>
    static void registerMessageType(std::string const& id)
    {
        static_assert(std::is_base_of<Message, T>::value, "Can only be used with types derived from ipc::Message");

        // Check for double registers
        if (m_ctors.find(id) != m_ctors.end())
            LESF_CORE_THROW(TypeException, "identifier `" << id << "` is already used");

        // Add the RTTI -> identifier entry
        m_rtti_map[typeid(T).name()] = id;

        // Create the constructor using a nice lambda
        m_ctors[id] = [](json::Node* data) -> Message* { return new T(data); };
    }

    // Get the identifier associated with a concrete message type. Throws an exception
    //   if the type is not registered.
    template <typename T>
    static std::string const& typeIdentifier()
    {
        auto it = m_rtti_map.find(typeid(T).name());

        if (it == m_rtti_map.end())
            LESF_CORE_THROW(TypeException, "type is not registered (RTTI name `" << typeid(T).name() << "`)");

        return it->second;
    }

    // Construct an IPC message instance from plain JSON data.
    // Throws an exception in the input data is not well formatted or if
    //   it used an unknown identifier.
    // Returns the identifier in *id if not null.
    static Message* construct(std::string const& json, std::string* id = 0);

    // Serialize an IPC message instance to JSON data. Throws an exception if the
    //   underlying class is not registered in the system.
    static std::string serialize(Message const& msg);

private:
    // Associates RTTI info of a type to its identifier in our system. We could
    //   implement equivalent functionnality without RTTI, but it's cleaner this way.
    static std::map<std::string, std::string> m_rtti_map;

    // Contains a constructor function for each registered type, taking parsed
    //   JSON representation as input to initialize data members.
    static std::map<std::string, std::function<Message*(json::Node*)>> m_ctors;
};

} }

#endif // __LESF_IPC_MESSAGE_FACTORY__
