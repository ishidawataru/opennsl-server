#!/usr/bin/make -f

SUBDIRS = pod std
include toc.make

HEADERS = $(wildcard *.hpp *.tpp)

DIST_FILES += \
	$(HEADERS) \
	createRegSerTraits.sh

SYMLINK_HEADERS = $(HEADERS)
HEADER_DIR = include/s11n.net/s11n/proxy
SYMLINK_HEADERS_DEST = $(top_srcdir)/$(HEADER_DIR)
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make

INSTALL_PACKAGE_HEADERS = $(HEADERS)
INSTALL_PACKAGE_HEADERS_DEST = $(prefix)/$(HEADER_DIR)
all: SYMLINK_HEADERS subdirs



