#!/usr/bin/make -f

include toc.make
ifeq ($(TOC_MAKE),)
toc.make:
	@echo "You must run configure once before running make."; exit 1
endif

SUBDIRS = include src doc
ifneq (,$(strip $(filter dist testdist,$(MAKECMDGOALS))))
    SUBDIRS += toc
endif

ifneq (,$(strip $(filter dist testdist clean distclean,$(MAKECMDGOALS))))
    SUBDIRS += example
endif



CLEAN_FILES += .toc* *~
DISTCLEAN_FILES += $(PACKAGE_NAME)-config $(PACKAGE_NAME).pc 

INSTALL_BINS = $(PACKAGE_NAME)-config
INSTALL_PKGCONFIG = $(PACKAGE_NAME).pc


DIST_FILES += \
	configure \
	configure.$(PACKAGE_NAME) \
	find_toc.sh \
	ChangeLog \
	TODO \
	README.$(PACKAGE_NAME) \
	LICENSE.$(PACKAGE_NAME) \
	toc.$(PACKAGE_NAME).make.at

all: subdirs
