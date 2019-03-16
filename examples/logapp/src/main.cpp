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

#include "lesf/lesf.h"

LESF_CONFIG_SYMBOLS()

using namespace lesf;

class MyLocalExceptionType : public core::RecoverableException
{
    using core::RecoverableException::RecoverableException;
};

int main(int argc, char** argv)
{
    log::Logger::init(argc, argv);

    LESF_LOG_TRACE("simple log " << 123456);

    try {
        std::function<void(int)> foo =
        [&](int l)
        {
            if (--l <= 0)
            {
                LESF_CORE_THROW(MyLocalExceptionType, "this is a bad exception " << 1234);
            }
            else
            {
                foo(l);
            }
        };

        foo(50);
    } catch (core::Exception const& exc) {
        LESF_LOG_ERROR_WITH_EXCEPT(exc, "log with exception");
    }
}
