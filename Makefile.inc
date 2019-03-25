PRODUCT   = libesf.so
VERSION   = 1.0
SUBDIRS   = examples/logserver examples/logapp examples/ipc_cmd

CC_FLAGS  = -fPIC -O0 -g -ggdb
CC_FLAGS += -Wno-unused-parameter
CC_FLAGS += -Icontrib/libconf/include
LD_FLAGS  = -shared
LD_FLAGS += -Lcontrib/libconf/bin -lconf
LD_FLAGS += -lboost_system -lboost_stacktrace_backtrace -ldl -lbacktrace -lrt -lpthread
