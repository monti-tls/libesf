PRODUCT   = logapp
VERSION   = 1.0
SUBDIRS   =

CC_FLAGS  = -O0 -g -ggdb
CC_FLAGS += -Wno-unused-parameter
CC_FLAGS += -I../../contrib/libconf/include -I../../inc
LD_FLAGS += -L../../bin -lesf