#!/usr/bin/make -f

include toc.make

HEADERS = $(wildcard *.hpp)

DIST_FILES += \
	$(HEADERS)

SYMLINK_HEADERS = $(HEADERS)
HEADER_DIR = include/s11n.net/s11n/proxy/std
SYMLINK_HEADERS_DEST = $(top_srcdir)/$(HEADER_DIR)
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make

INSTALL_PACKAGE_HEADERS = $(HEADERS)
INSTALL_PACKAGE_HEADERS_DEST = $(prefix)/$(HEADER_DIR)
all: SYMLINK_HEADERS


