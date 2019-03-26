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

#ifndef __LESF_IPC_USER_ACTIONS_H__
#define __LESF_IPC_USER_ACTIONS_H__

#ifndef LESF_IPC_USER_ACTIONS_DEF
# error "User must provide the commands .def file using LESF_IPC_USER_ACTIONS_DEF"
#endif

#include <string>
#include <functional>
#include <set>
#include <map>
#include <mutex>

#include "lconf/json.h"

#include "lesf/ipc/action_server.h"
#include "lesf/ipc/message.h"
#include "lesf/ipc/exception.h"
#include "lesf/core/preprocessor.h"

/* The x-macro system below allows automatic generation of boilerplate classes
 *  for interfacing with ipc::ActionServer and providing an asynchronous action/response
 *  interface over an ipc::Endpoint.
 * Users of the library should define their actions in a user.def file, and define the
 *  LESF_IPC_USER_ACTIONS_DEF accordingly.
 *
 * Please note that users should also include lesf/ipc/user_actions_symbols.h once in a
 *   compiled module in order to instanciate static class symbols.
 */

#define DECLARE_IMPL__(_type, _name) \
    _type _name;

#define DECLARE_IMPL_EACH(_decl) \
    DECLARE_IMPL_ ## _decl

#define DECLARE_FOREACH(...) \
    __VA_OPT__( \
        LESF_CORE_PREPROCESSOR_EVAL( \
            LESF_CORE_PREPROCESSOR_MAP( \
                DECLARE_IMPL_EACH, \
                __VA_ARGS__)))

#define DECLARE_PARAMS(...) \
    DECLARE_FOREACH(__VA_ARGS__)

#define DECLARE_RESPONSE(...) \
    DECLARE_FOREACH(__VA_ARGS__)

#define DECLARE(_params_or_response) \
    DECLARE_ ## _params_or_response

/////////

#define REFLIST_IMPL__(_type, _name) \
    , data._name

#define REFLIST_IMPL_EACH(_decl) \
    REFLIST_IMPL_ ## _decl

#define REFLIST_FOREACH(...) \
    __VA_OPT__( \
        LESF_CORE_PREPROCESSOR2_EVAL( \
            LESF_CORE_PREPROCESSOR2_MAP( \
                REFLIST_IMPL_EACH, \
                __VA_ARGS__)))

#define REFLIST_PARAMS(...) \
    REFLIST_FOREACH(__VA_ARGS__)

#define REFLIST_RESPONSE(...) \
    REFLIST_FOREACH(__VA_ARGS__)

#define REFLIST(_params_or_response) \
    REFLIST_ ## _params_or_response

/////////

#define BINDINGS_IMPL__(_type, _name) \
    tpl.bind(#_name, data._name);

#define BINDINGS_IMPL_EACH(_decl) \
    BINDINGS_IMPL_ ## _decl

#define BINDINGS_FOREACH(...) \
    __VA_OPT__( \
        LESF_CORE_PREPROCESSOR2_EVAL( \
            LESF_CORE_PREPROCESSOR2_MAP( \
                BINDINGS_IMPL_EACH, \
                __VA_ARGS__)))

#define BINDINGS_PARAMS(...) \
    BINDINGS_FOREACH(__VA_ARGS__)

#define BINDINGS_RESPONSE(...) \
    BINDINGS_FOREACH(__VA_ARGS__)

#define BINDINGS(_params_or_response) \
    BINDINGS_ ## _params_or_response

/////////

#define ACTION(_ns, _name, _params, _response) \
namespace lesf { namespace ipc { namespace user { namespace _ns { \
    class _name \
    { \
        friend class lesf::ipc::ActionServer; \
        \
    public: \
        struct Params \
        { \
            DECLARE(_params) \
        }; \
        struct Response \
        { \
            DECLARE(_response) \
        }; \
        struct Error \
        { \
            Error() : \
                set(false), message(), code(0) \
            {} \
            Error(std::string const& message, int code = -1) : \
                set(true), message(message), code(code) \
            {} \
            bool set; \
            std::string message; \
            int code; \
        }; \
        \
        typedef std::function<void(ResponseOrError<_name> const&, std::string const&)> ResponseHandler; \
        \
    private: \
        class ActionData : public ipc::Message \
        { \
            LESF_IPC_MESSAGE(ActionData) \
            LESF_IPC_MEMBERS(id REFLIST(_params)) \
            \
        public: \
            ActionData(std::string const& id, Params const& data) : \
                id(id), \
                data(data) \
            {} \
            \
        public: \
            std::string id; \
            Params data; \
        }; \
        \
        class ResponseData : public ipc::Message \
        { \
            LESF_IPC_MESSAGE(ResponseData) \
            LESF_IPC_MEMBERS(id, error.set, error.message, error.code REFLIST(_response)) \
        public: \
            ResponseData(std::string const& id, Response const& data) : \
                id(id), \
                error(), \
                data(data) \
            {} \
            ResponseData(std::string const& id, Error const& error) : \
                id(id), \
                error(error), \
                data{} \
            {} \
            \
        public: \
            std::string id; \
            Error error; \
            Response data; \
        }; \
        \
        class Internals \
        { \
        public: \
            Internals() \
            { \
                MessageFactory::registerMessageType<ActionData>(#_ns "::" #_name "_action"); \
                MessageFactory::registerMessageType<ResponseData>(#_ns "::" #_name "_response"); \
            } \
            \
            std::set<Endpoint*> endpoints; \
            std::map<std::string, ResponseHandler> active_handlers; \
        }; \
    \
    public: \
        _name(Params const& params) : \
            m_params(params) \
        {} \
        \
        _name(Params&& params) : \
            m_params(std::move(params)) \
        {} \
        \
        ~_name() \
        {} \
        \
        std::string async(Endpoint* ep, ResponseHandler handler) \
        { \
            std::lock_guard<std::mutex> lock(m_mutex); \
             \
            ActionData data(ActionServer::generateId(), m_params); \
            \
            if (m_internals.endpoints.find(ep) == m_internals.endpoints.end()) \
            { \
                m_internals.endpoints.insert(ep); \
                ep->registerSlot<_name::ResponseData>( \
                    std::bind(&_name::M_responseHandler, \
                              static_cast<_name*>(0), \
                              std::placeholders::_1, \
                              std::placeholders::_2)); \
            } \
            \
            m_internals.active_handlers[data.id] = handler; \
            ep->send(data); \
            \
            return data.id; \
        } \
        \
        static Params constructParams(std::string const& json) \
        { \
            try { \
                Params data; \
                \
                json::Template tpl; \
                BINDINGS(_params) \
                if (!tpl.bound()) \
                    return Params{}; \
                std::istringstream ss(json); \
                json::Node* repr = json::parse(ss); \
                tpl.extract(repr); \
                delete repr; \
                \
                return std::move(data); \
            } catch (std::exception const& exc) { \
                LESF_CORE_THROW(DataFormatException, "unable to construct parameters from JSON data for action " #_ns "::" #_name ": " << exc.what()); \
            } \
        } \
        \
        static std::string serializeResponse(Response const& resp) \
        { \
            try { \
                Response& __attribute__((unused)) data = const_cast<Response&>(resp); \
                \
                json::Template tpl; \
                BINDINGS(_response) \
                \
                if (!tpl.bound()) \
                    return "{}"; \
                std::ostringstream ss; \
                json::Node* repr = tpl.synthetize(); \
                repr->serialize(ss, true); \
                delete repr; \
                \
                return ss.str(); \
            } catch (std::exception const& exc) { \
                LESF_CORE_THROW(DataFormatException, "unable to serialize response for action " #_ns "::" #_name); \
            } \
        } \
        \
        static std::string serializeError(Error const& err) \
        { \
            try { \
                Error& data = const_cast<Error&>(err); \
                \
                json::Template tpl; \
                tpl.bind("message", data.message); \
                tpl.bind("code", data.code); \
                \
                std::ostringstream ss; \
                json::Node* repr = tpl.synthetize(); \
                repr->serialize(ss, true); \
                delete repr; \
                \
                return ss.str(); \
            } catch (std::exception const& exc) { \
                LESF_CORE_THROW(DataFormatException, "unable to serialize error for action " #_ns "::" #_name); \
            } \
        } \
        static std::string serializeResponseOrError(ResponseOrError<_name> const& roe)\
        { \
            if (roe.isError()) \
                return serializeError(roe.getError()); \
            else \
                return serializeResponse(roe.getResponse()); \
        } \
    private: \
        void M_responseHandler(Endpoint&, ResponseData const& resp) \
        { \
            std::lock_guard<std::mutex> lock(m_mutex); \
            \
            auto it = m_internals.active_handlers.find(resp.id); \
            \
            if (it == m_internals.active_handlers.end()) \
            { \
                LESF_CORE_THROW(BadActionId, "unknown response id in command " #_ns "::" #_name " (" << resp.id << ")"); \
                return; \
            } \
            \
            std::shared_ptr<ResponseOrError<_name>> resp_or_err; \
            if (resp.error.set) \
                resp_or_err = std::make_shared<ResponseOrError<_name>>(resp.error); \
            else \
                resp_or_err = std::make_shared<ResponseOrError<_name>>(resp.data); \
            it->second(*resp_or_err, resp.id); \
            m_internals.active_handlers.erase(it); \
        } \
    \
    private: \
        static std::mutex m_mutex; \
        static Internals m_internals; \
        Params m_params; \
    }; \
} } } }

#include LESF_IPC_USER_ACTIONS_DEF

#undef DECLARE_IMPL__
#undef DECLARE_IMPL_EACH
#undef DECLARE_FOREACH
#undef DECLARE_PARAMS
#undef DECLARE_RESPONSE
#undef DECLARE
#undef REFLIST_IMPL__
#undef REFLIST_IMPL_EACH
#undef REFLIST_FOREACH
#undef REFLIST_PARAMS
#undef REFLIST_RESPONSE
#undef REFLIST
#undef BINDINGS_IMPL__
#undef BINDINGS_IMPL_EACH
#undef BINDINGS_FOREACH
#undef BINDINGS_PARAMS
#undef BINDINGS_RESPONSE
#undef BINDINGS
#undef ACTION

#endif // __LESF_IPC_USER_ACTIONS_H__
