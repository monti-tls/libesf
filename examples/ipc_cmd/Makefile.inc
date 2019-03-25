PRODUCT   = ipc_cmd
VERSION   = 1.0
SUBDIRS   =

DEFINES  += -DLESF_IPC_USER_ACTIONS_DEF='"ipc_actions.def"'

CC_FLAGS  = -O0 -g -ggdb
CC_FLAGS += -Wno-unused-parameter
CC_FLAGS += -I../../contrib/libconf/include -I../../inc
LD_FLAGS += -L../../bin -lesf -lpthread
LD_FLAGS += -Wl,--rpath $(shell pwd)/../../bin
