PRODUCT   = ipc_cmd
VERSION   = 1.0
SUBDIRS   =

DEFINES  += -DLESF_IPC_USER_ACTIONS_DEF='"ipc_actions.def"'

CC_FLAGS  = -O0 -g -ggdb
CC_FLAGS += -Wno-unused-parameter
CC_FLAGS += -I../../contrib/libconf/include -I../../inc

LD_FLAGS += -Wl,-Bstatic
LD_FLAGS += -L../../bin -lesf -L../../contrib/libconf/bin -lconf
LD_FLAGS += -L../../../../../build_root/usr/lib -lboost_stacktrace_backtrace -lbacktrace 
LD_FLAGS += -Wl,-Bdynamic
LD_FLAGS += -ldl -lpthread -lrt
