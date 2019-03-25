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

#include "lesf/ipc/action_server.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/algorithm/hex.hpp>

using namespace lesf::ipc;

ActionServer::ActionServer(Endpoint& ep) :
    m_ep(ep)
{}

ActionServer::~ActionServer()
{}

std::string ActionServer::generateId()
{
    boost::uuids::uuid u = boost::uuids::random_generator()();
    std::string as_hex;
    boost::algorithm::hex(u.begin(), u.end(), std::back_inserter(as_hex));
    return std::move(as_hex);
}
