PRODUCT   = libesf
VERSION   = 1.0
SUBDIRS   = examples/logserver examples/logapp examples/ipc_cmd

DIST_DIR  = lib

CC_FLAGS  = -O0 -g -ggdb
CC_FLAGS  = -fPIC
CC_FLAGS += -Wno-unused-parameter

CC_FLAGS += -I../../../build_root/usr/include
CC_FLAGS += -Icontrib/libconf/include
LD_FLAGS += -L../../../build_root/usr/lib
LD_FLAGS += -Lcontrib/libconf/bin -lconf
LD_FLAGS += -Wl,-Bstatic
LD_FLAGS += -lboost_system -lboost_stacktrace_backtrace -lbacktrace
LD_FLAGS += -Wl,-Bdynamic
LD_FLAGS += -ldl -lrt -lpthread
