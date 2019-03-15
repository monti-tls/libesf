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

#ifndef __LESF_LOG_FORMATTER_H__
#define __LESF_LOG_FORMATTER_H__

#include "lesf/log/message.h"

#include <iostream>

namespace lesf { namespace log {

class Formatter
{
public:
    Formatter(std::ostream& os);
    virtual ~Formatter();

    virtual void operator()(log::Message const& msg) = 0;

protected:
    std::ostream& M_os();

private:
    std::ostream& m_os;
};

namespace formatters {

    class raw : public Formatter
    {
    public:
        using Formatter::Formatter;

        void operator()(log::Message const& msg);
    };

    class plain_light : public Formatter
    {
    public:
        using Formatter::Formatter;

        void operator()(log::Message const& msg);
    };

}

} }

#endif // __LESF_LOG_FORMATTER_H__
