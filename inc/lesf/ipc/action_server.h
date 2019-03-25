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

#ifndef __LESF_IPC_ACTION_SERVER_H__
#define __LESF_IPC_ACTION_SERVER_H__

#include "lesf/ipc/endpoint.h"

#include <string>
#include <functional>
#include <thread>

namespace lesf { namespace ipc {

class ActionServer
{
public:
    ActionServer(Endpoint& ep);
    ~ActionServer();

    template <typename T>
    void registerAction(std::function<std::function<typename T::Response(typename T::Params const&, std::string const&)>()> handler_factory)
    {
        m_ep.registerSlot<typename T::ActionData>(
            [handler_factory](Endpoint& ep, typename T::ActionData const& action)
            {
                std::thread([&ep, handler_factory, action]()
                {
                    auto user_handler = handler_factory();
                    ep.send(typename T::ResponseData(action.id, handler_factory()(action.data, action.id)));
                }).detach();
            });
    }

    static std::string generateId();

private:
    Endpoint& m_ep;
};

} }

#endif // __LESF_IPC_ACTION_SERVER_H__
