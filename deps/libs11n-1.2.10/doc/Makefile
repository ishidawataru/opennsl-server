#!/usr/bin/make -f
include toc.make

SUBDIRS = manual
ifneq (,$(DOXYGEN_BIN))
  SUBDIRS += doxygen
endif


all: subdirs

