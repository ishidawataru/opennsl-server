#!/usr/bin/make -f

# NOTE: libshellish must be installed before this makefile will work.

include toc.make


ECONFIG = $(prefix)/bin/libshellish-config
ifeq (,$(wildcard $(ECONFIG)))
    $(warning Error: $(ECONFIG) not found)
all:
else

INCLUDES = -I. $(shell $(ECONFIG) --includes)

BIN_PROGRAMS = example
BIN_PROGRAMS_LDADD = $(shell $(ECONFIG) --libs)
example_bin_OBJECTS = main.o
include $(TOC_MAKESDIR)/BIN_PROGRAMS.make

all: BIN_PROGRAMS

endif
# ^^^ got libshellish-config?
