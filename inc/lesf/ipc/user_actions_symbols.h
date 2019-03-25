#ifndef __LESF_IPC_USER_ACTIONS_SYMBOLS_H__
#define __LESF_IPC_USER_ACTIONS_SYMBOLS_H__

#ifndef LESF_IPC_USER_ACTIONS_DEF
# error "User must provide the commands .def file using LESF_IPC_USER_ACTIONS_DEF"
#endif

#include "lesf/core/preprocessor.h"

#define ACTION(_ns, _name, _params, _response) \
    std::mutex lesf::ipc::user::_ns::_name::m_mutex; \
    lesf::ipc::user::_ns::_name::Internals lesf::ipc::user::_ns::_name::m_internals;

#include LESF_IPC_USER_ACTIONS_DEF

#undef ACTION

#endif // __LESF_IPC_USER_ACTIONS_SYMBOLS_H__
