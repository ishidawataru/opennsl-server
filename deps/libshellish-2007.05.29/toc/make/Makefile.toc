#!/usr/bin/make -f
default: all
include toc.make

DIST_FILES += $(wildcard *.make *.at makerules.*)

