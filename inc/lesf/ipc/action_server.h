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

namespace detail {
    template <typename U, typename V>
    class Either
    {
    private:
        template <typename T> struct type {};

    public:
        Either(U&& u) :
            m_u(std::make_shared<U>(std::move(u))),
            m_v(0)
        {}

        Either(U const& u) :
            m_u(std::make_shared<U>(u)),
            m_v(0)
        {}

        Either(V&& v) :
            m_u(0),
            m_v(std::make_shared<V>(std::move(v)))
        {}

        Either(V const& v) :
            m_u(0),
            m_v(std::make_shared<V>(v))
        {}


        template <typename T>
        bool is() const
        { return M_is(type<T>()); }

        template <typename T>
        T const& get() const
        { return M_get(type<T>()); }

    private:
        template <typename T>
        bool M_is(type<T>) const
        { return false; }

        bool M_is(type<U>) const
        { return m_u != nullptr; }

        bool M_is(type<V>) const
        { return m_v != nullptr; }

        template <typename T>
        T const& M_get(type<T>) const
        { return *static_cast<T const*>(0); }

        U const& M_get(type<U>) const
        { return *m_u; }

        V const& M_get(type<V>) const
        { return *m_v; }

    private:
        std::shared_ptr<U> m_u;
        std::shared_ptr<V> m_v;
    };
}

template <typename T>
class ResponseOrError : public detail::Either<typename T::Error,
                                              typename T::Response>
{
    using detail::Either<typename T::Error, typename T::Response>::Either;

public:
    bool isResponse() const
    { return this->template is<typename T::Response>(); }

    bool isError() const
    { return this->template is<typename T::Error>(); }

    typename T::Response const& getResponse() const
    { return this->template get<typename T::Response>(); }

    typename T::Error const& getError() const
    { return this->template get<typename T::Error>(); }
};

class ActionServer
{
public:
    ActionServer(Endpoint& ep);
    ~ActionServer();

    template <typename T>
    void registerAction(std::function<ResponseOrError<T>(typename T::Params const&, std::string const&)> handler)
    {
        m_ep.registerSlot<typename T::ActionData>(
            [handler](Endpoint& ep, typename T::ActionData const& action)
            {
                std::thread([&ep, handler, action]()
                {
                    auto user_res = handler(action.data, action.id);

                    if (user_res.template is<typename T::Error>())
                        ep.send(typename T::ResponseData(action.id, user_res.template get<typename T::Error>()));
                    else
                        ep.send(typename T::ResponseData(action.id, user_res.template get<typename T::Response>()));
                }).detach();
            });
    }

    static std::string generateId();

private:
    Endpoint& m_ep;
};

} }

#endif // __LESF_IPC_ACTION_SERVER_H__
