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

#include "lesf/log/message.h"
#include "lconf/json.h"

#include <boost/algorithm/hex.hpp>

#include <memory>
#include <array>

namespace lconf { namespace json {

template <typename Clock, typename Duration>
class TimePointElement : public UserElement
{
    typedef std::chrono::time_point<Clock, Duration> type_t;
public:
    TimePointElement(type_t& ref) :
        m_ref(ref)
    {}

    void extract(Node* node) const
    {
        std::string as_hex = node->downcast<StringNode>()->value();

        std::vector<char> raw;
        boost::algorithm::unhex(as_hex.begin(), as_hex.end(), std::back_inserter(raw));

        if (raw.size() != sizeof(typename type_t::rep))
            throw lconf::json::Exception(node, "invalid size");

        typename type_t::rep rep;
        memcpy(reinterpret_cast<char*>(&rep), raw.data(), sizeof(rep));

        m_ref = type_t{Duration(rep)};
    }

    Node* synthetize() const
    {
        std::array<char, sizeof(typename type_t::rep)> raw;
        auto rep = m_ref.time_since_epoch().count();

        memcpy(raw.data(), reinterpret_cast<const char*>(&rep), sizeof(rep));

        std::string as_hex;
        boost::algorithm::hex(raw.begin(), raw.end(), std::back_inserter(as_hex));

        return new StringNode(as_hex);
    }

private:
    type_t& m_ref;
};

template <typename Clock, typename Duration>
class Terminal<std::chrono::time_point<Clock, Duration>> : public TimePointElement<Clock, Duration>
{
    using TimePointElement<Clock, Duration>::TimePointElement;
};

} }

using namespace lconf;

using namespace lesf;
using namespace log;

Message::Message(Message::Type type, Message::BuildInfo&& build_info,
                 Message::ProcessInfo&& process_info, std::string const& message) :
    m_type(type),
    m_build_info(std::move(build_info)),
    m_process_info(std::move(process_info)),
    m_time(std::chrono::system_clock::now()),
    m_message(message),
    m_exception_info(nullptr)
{}

Message::Message(Message::Type type, Message::BuildInfo&& build_info,
                 Message::ProcessInfo&& process_info, std::string const& message,
                 core::Exception const& exception) :
    Message(type, std::move(build_info), std::move(process_info), message)
{
    m_exception_info = std::make_shared<ExceptionInfo>();

    m_exception_info->what  = exception.what();
    m_exception_info->rtti_type = typeid(exception).name();
    m_exception_info->build_info = exception.buildInfo().set ?
    ExceptionInfo::BuildInfo{
        exception.buildInfo().file,
        exception.buildInfo().line,
        exception.buildInfo().function,
        exception.buildInfo().build_date,
        exception.buildInfo().build_time,
        exception.buildInfo().user_build_id,
        exception.buildInfo().user_program,
        exception.buildInfo().user_version
    } : ExceptionInfo::BuildInfo{};

    if (exception.debugInfo().set)
    {
        std::ostringstream ss;
        ss << exception.debugInfo().trace;
        std::string trace = ss.str();

        boost::algorithm::hex(trace.begin(), trace.end(), std::back_inserter(m_exception_info->trace));
    }
}

Message::~Message()
{}

Message::Type Message::type() const
{
    return m_type;
}

Message::BuildInfo const& Message::buildInfo() const
{
    return m_build_info;
}

Message::ProcessInfo const& Message::processInfo() const
{
    return m_process_info;
}

std::string const& Message::message() const
{
    return m_message;
}

std::chrono::system_clock::time_point const& Message::time() const
{
    return m_time;
}

Message::ExceptionInfoPtr Message::exceptionInfo() const
{
    return m_exception_info;
}

std::string Message::serialize(Message const& msg) noexcept
{
    try {
        // Construct the JSON template for the message
        //   (aarg, this const_cast is ugly, I should update lconf::json::Template
        //   to accept const references)
        json::Template tpl = const_cast<Message&>(msg).M_jsonTemplate();
        
        // Try to include the exception info, if any
        bool has_exception_info = (bool) msg.m_exception_info;

        json::Template tpl_exception_info_wrapper;
        tpl_exception_info_wrapper.bind("set", has_exception_info);

        if (has_exception_info)
            tpl_exception_info_wrapper.bind("data", const_cast<Message&>(msg).M_exceptionInfoJsonTemplate());

        tpl.bind("exception_info", tpl_exception_info_wrapper);

        // Generate JSON representation of the log message
        json::Node* repr = tpl.synthetize();

        // Convert JSON representation to string and return
        std::ostringstream ss;
        repr->serialize(ss, false);
        delete repr;
        return ss.str();
    } catch(std::exception const& exc) {
        try {
            return std::string("$ exception caught during log message serialization: '") + exc.what() + "'";
        } catch (std::exception const&) {
            return "$ exception caught during log message serialization, unable to include message";
        }
    }
}

Message Message::synthetize(std::string const& serialized) noexcept
{
    try {
        Message msg(Message::Unknown, {}, {}, "");

        // Create the basic JSON template (without exception info)
        json::Template tpl = msg.M_jsonTemplate();
        json::Template tpl_exception_info_wrapper;
        bool has_exception_info;
        tpl_exception_info_wrapper.bind("set", has_exception_info);
        tpl.bind("exception_info", tpl_exception_info_wrapper);

        // Parse the input data and extract the template
        std::istringstream ss(serialized);
        json::ObjectNode* repr = json::parse(ss)->downcast<json::ObjectNode>();
        tpl.extract(repr);

        // Check if exception info was present
        if (has_exception_info)
        {
            msg.m_exception_info = std::make_shared<ExceptionInfo>();

            json::Template tpl_exception_info = msg.M_exceptionInfoJsonTemplate();

            json::Node* repr_exception_info =
                repr->get("exception_info")->downcast<json::ObjectNode>()
                    ->get("data");

            tpl_exception_info.extract(repr_exception_info);
        }

        delete repr;
        return msg;
    } catch (std::exception const& exc) {
        std::ostringstream ss;
        ss << "<log message synthesis failed: " << exc.what() << ">\nraw data: " << serialized;
        return Message(Message::Unknown, {}, {}, ss.str());
    }
}

json::Template Message::M_jsonTemplate()
{
    json::Template tpl;
    tpl.bind("type", (int&) m_type);
    tpl.bind("build_info", M_buildInfoJsonTemplate());
    tpl.bind("process_info", M_processInfoJsonTemplate());
    tpl.bind("time", m_time);
    tpl.bind("message", m_message);

    return tpl;
}

json::Template Message::M_buildInfoJsonTemplate()
{
    json::Template tpl;
    tpl.bind("file", m_build_info.file);
    tpl.bind("line", m_build_info.line);
    tpl.bind("function", m_build_info.function);
    tpl.bind("build_date", m_build_info.build_date);
    tpl.bind("build_time", m_build_info.build_time);
    tpl.bind("user_build_id", m_build_info.user_build_id);
    tpl.bind("user_program", m_build_info.user_program);
    tpl.bind("user_version", m_build_info.user_version);
    return tpl;
}

json::Template Message::M_processInfoJsonTemplate()
{
    json::Template tpl;
    tpl.bind("process", m_process_info.process);
    tpl.bind("pid", m_process_info.pid);
    return tpl;
}

json::Template Message::M_exceptionInfoJsonTemplate()
{
    json::Template tpl;

    if (!m_exception_info) // should not happen!
        return tpl;

    json::Template tpl_build_info;
    tpl_build_info.bind("file", m_exception_info->build_info.file);
    tpl_build_info.bind("line", m_exception_info->build_info.line);
    tpl_build_info.bind("function", m_exception_info->build_info.function);
    tpl_build_info.bind("build_date", m_exception_info->build_info.build_date);
    tpl_build_info.bind("build_time", m_exception_info->build_info.build_time);
    tpl_build_info.bind("user_build_id", m_exception_info->build_info.user_build_id);
    tpl_build_info.bind("user_program", m_exception_info->build_info.user_program);
    tpl_build_info.bind("user_version", m_exception_info->build_info.user_version);

    tpl.bind("what", m_exception_info->what);
    tpl.bind("rtti_type", m_exception_info->rtti_type);
    tpl.bind("build_info", tpl_build_info);
    tpl.bind("trace", m_exception_info->trace);

    return tpl;
}
