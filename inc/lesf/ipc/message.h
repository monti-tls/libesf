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

#ifndef __LESF_IPC_MESSAGE_H__
#define __LESF_IPC_MESSAGE_H__

#include "lconf/json.h"
#include "lesf/ipc/preprocessor_magic.h"

namespace lesf { namespace ipc {

using namespace lconf;

// The macros defined here are a bit QObject-style and can be used to
//   reduce overhead code to a minimum when it comes to define your own
//   concrete ipc::Message types.
// use them like this :
//
// class FooBarMessage : ipc::Message {
//     LESF_IPC_MESSAGE(FooBarMessage)
//     LESF_IPC_MEMBERS(m_a, m_b, m_c)
//     // ... rest of your code as usual
// }
//
// As long as specified members are compatible with libconf's type system,
//   everything is fine.
// Do NOT use any LESF_IPC_MAGIC_* macros, they're internal fuckery.

#define LESF_IPC_MESSAGE(class_name) \
public: \
    class_name(lconf::json::Node* data) { ipc::Message::M_pull(data); }

#define LESF_IPC_MAGIC_MEMBER(arg) \
    tpl.bind(#arg, arg);

#define LESF_IPC_MEMBERS(...) \
protected: \
    lconf::json::Template M_jsonTemplate() { \
        lconf::json::Template tpl; \
        __VA_OPT__(LESF_IPC_MAGIC_EVAL(LESF_IPC_MAGIC_MAP(LESF_IPC_MAGIC_MEMBER, __VA_ARGS__))) \
        return tpl; }

// Base class for IPC messages with typed auto-serialiation and synthesis.
// All concrete IPC message classes must implement the M_jsonTemplate()
//   method in order to expose their data members to the system.
class Message
{
    friend class MessageFactory; // to allow access to M_push()

public:
    virtual ~Message()
    {}

protected:
    // Overloaded by concrete message classes in order to register their
    //   data members for serialization & synthesis.
    virtual json::Template M_jsonTemplate() = 0;

    // Pull data members from parsed JSON.
    void M_pull(json::Node* data)
    {
        json::Template tpl = M_jsonTemplate();

        // Allow empty messages, in this case we must avoid extracting the
        //   template bc it throws an exception
        if (tpl.bound())
            tpl.extract(data);
    }

    // Push data members into a JSON representation.
    json::Node* M_push() const
    {
        // I know, this const_cast is ugly, but...
        json::Template tpl = const_cast<Message*>(this)->M_jsonTemplate();

        if (tpl.bound())
            return tpl.synthetize();

        return 0;
    }
};

} }

#endif // __LESF_IPC_MESSAGE_H__
