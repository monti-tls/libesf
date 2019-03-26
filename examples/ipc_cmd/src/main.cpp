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
#include <functional>
#include <algorithm>
#include <string>
#include <thread>
#include <chrono>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/algorithm/hex.hpp>

#include "lesf/lesf.h"
#include "lesf/ipc/action_server.h"
#include "lesf/ipc/user_actions.h"
#include "lesf/ipc/user_actions_symbols.h"

LESF_CONFIG_SYMBOLS()

using namespace lesf::ipc;
using namespace lesf::log;

int main(int argc, char** argv)
{
    Logger::init(argc, argv);

    using namespace user::camera;

    ///// Server ///// 
    
    Endpoint* srv_ep = new Endpoint(Endpoint::Server, "ipc_cmd");
    ActionServer* srv = new ActionServer(*srv_ep);

    int count = 0;
    srv->registerAction<SetZoom>(
        [&count](SetZoom::Params const& params, std::string const&)
            -> ResponseOrError<SetZoom>
        {
            std::cout << "Processing action (foo=" << params.foo << ", bar=" << params.bar << ")" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            std::cout << "Done!" << std::endl;
            if (count++ == 0)
            {
                return SetZoom::Response
                {
                    .baz =true,
                    .bat = 453.12
                };
            }
            else
            {
                return SetZoom::Error
                {
                    .set = true,
                    .code = 0,
                    .message = "YOLO"
                };
            }
        });

    ///// Client /////

    auto ep = new Endpoint(Endpoint::Client, "ipc_cmd");
    int done = 0;

    SetZoom({123, "banana"}).async(ep,
        [&done](ResponseOrError<SetZoom> const& resp_or_err, std::string const& id)
        {
            if (resp_or_err.isError())
                std::cout << "[" << id << "] error: " << resp_or_err.getError().message << std::endl;
            else
            {
                auto resp = resp_or_err.getResponse();
                std::cout << "[" << id << "] " << resp.baz << " (" << resp.bat << ")" << std::endl;
            }
            ++done;
        });

    SetZoom({666, "pineapple"}).async(ep,
        [&done](ResponseOrError<SetZoom> const& resp_or_err, std::string const& id)
        {
            if (resp_or_err.isError())
                std::cout << "[" << id << "]: error: " << resp_or_err.getError().message << std::endl;
            else
            {
                auto resp = resp_or_err.getResponse();
                std::cout << "[" << id << "] " << resp.baz << " (" << resp.bat << ")" << std::endl;
            }
            ++done;
        });

    while (done != 2);

    delete ep;
    delete srv;
    delete srv_ep;
}
