#ifndef __LESF_CONFIG_H__
#define __LESF_CONFIG_H__

#include "lesf/log/config.h"
#include "lesf/daemon/config.h"

#define LESF_CONFIG_SYMBOLS() \
    LESF_LOG_CONFIG_SYMBOLS() \
    LESF_DAEMON_CONFIG_SYMBOLS()

#endif // __LESF_CONFIG_H__
