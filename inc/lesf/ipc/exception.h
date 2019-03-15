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

#ifndef __LESF_IPC_EXCEPTION_H__
#define __LESF_IPC_EXCEPTION_H__

#include <stdexcept>
#include <sstream>

#include "lesf/core/exception.h"

namespace lesf { namespace ipc {

class SharedMemoryException : public core::RecoverableException
{
    using core::RecoverableException::RecoverableException;
};

class TypeException : public core::RecoverableException
{
    using core::RecoverableException::RecoverableException;
};

class DataFormatException : public core::RecoverableException
{
    using core::RecoverableException::RecoverableException;
};

} }

#endif // __LESF_IPC_EXCEPTION_H__
