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

#include "lesf/ipc/message_factory.h"

using namespace lconf;

using namespace lesf;
using namespace ipc;

std::map<std::string, std::string> MessageFactory::m_rtti_map;
std::map<std::string, std::function<Message*(json::Node*)>> MessageFactory::m_ctors;

Message* MessageFactory::construct(std::string const& json, std::string* id)
{
    // Parse the JSON input
    std::istringstream ss;
    ss.str(json);

    json::Node* data;
    try {
        data = json::parse(ss);
    } catch (json::Exception const& exc) {
        LESF_CORE_THROW(DataFormatException, "invalid IPC JSON data (" << exc.what() << ")");
    }

    // Check JSON structure, get command name and payload
    json::ObjectNode* obj;
    json::Node* idNode;
    json::StringNode* idStringNode;
    json::Node* payload;
    if (!(obj = data->downcast<json::ObjectNode>()) ||
        !(idNode = obj->get("id")) ||
        !(idStringNode = idNode->downcast<json::StringNode>()) ||
        !(payload = obj->get("payload")))
    {
        LESF_CORE_THROW(DataFormatException, "invalid IPC JSON data");
    }

    // Find the associated constructor
    auto it = m_ctors.find(idStringNode->value());
    if (it == m_ctors.end())
        LESF_CORE_THROW(DataFormatException, "unknown identifier `" << idStringNode->value() << "` in IPC JSON data");

    // If necessary, get the type identifier
    if (id)
        *id = idStringNode->value();

    // Construct the IPC message and pull data from JSON representation
    Message* msg;
    try {
        msg = it->second(payload);
    } catch(json::Exception const& exc) {
        LESF_CORE_THROW(DataFormatException, "IPC JSON data is not consistent with data member bindings for type `" << idStringNode->value() << "` : " << exc.what());
    }

    delete data;
    return msg;
}

std::string MessageFactory::serialize(Message const& msg)
{
    // Check if the message type is registered in the system
    auto rtti_id = typeid(msg).name();
    if (m_rtti_map.find(rtti_id) == m_rtti_map.end())
        LESF_CORE_THROW(TypeException, "type is not registered (RTTI name `" << rtti_id << "`)");
    
    // Get the associated IPC identifier
    auto id = m_rtti_map[rtti_id];

    // Create the JSON representation
    json::Template tpl;
    tpl.bind("id", id);
    json::ObjectNode* data = tpl.synthetize()->downcast<json::ObjectNode>();
    json::Node* payload_data = msg.M_push();
    data->get("payload") = payload_data ? payload_data : new json::ObjectNode();

    // Serialize it into plain text and return
    std::ostringstream ss;
    data->serialize(ss, false);
    delete data;
    return ss.str();
}
