#!/bin/make
#
# toc.make.at: template makefile for toc.make, a core concept of the
# toc build process. This is filtered at the end of the configure
# process. toc.make must be included by your Makefiles.
#
# All of the "at-vars" in this file are expected to come in
# via the configure process, either from core tests or from
# the core itself.
#
# Ideally this file should be free of project-specific code:
# put that in $(top_srcdir)/toc.$(PACKAGE_NAME).make.at and
# in $(top_srcdir)/configure.$(PACKAGE_NAME) run:
#   toc_test_require toc_project_makefile


SHELL = /bin/bash

default: all
all:
FORCE: ; @true

ifneq (,$(wildcard Makefile.toc))
  TOC_MAKEFILE_NAME = Makefile.toc
else
  TOC_MAKEFILE_NAME = Makefile
endif

# try to save some grief later:
.toc.%: $(wildcard *)

ifneq (,$(strip $(filter distclean clean,$(MAKECMDGOALS))))
    MAKING_CLEAN = 1
else
    MAKING_CLEAN = 0
endif

#PACKAGE_VERSION = 2007.05.29
#PACKAGE_NAME = libshellish

top_srcdir = ../..

retoc:
	@echo "Cleaning up .toc.*"; ls="$$(ls .toc.* 2>/dev/null)"; test x = "x$ls" && exit 0; \
	rm $$ls; ${MAKE} $$ls


PACKAGE_NAME = libshellish
##### include configure-created code:
toc_config_vars = $(top_srcdir)/toc.$(PACKAGE_NAME).configure.make
include $(toc_config_vars)

##### DESTDIR is for autotools 'make install' compatibility
ifneq (,$(DESTDIR))
  DESTDIR := $(DESTDIR)/
endif

##### some conventional vars:
prefix ?= /usr
top_includesdir = $(top_srcdir)/include
top_libdir = $(top_srcdir)/lib
top_bindir = $(top_srcdir)/bin




toc_project_makefile = $(wildcard $(top_srcdir)/toc.$(PACKAGE_NAME).make)
toc_project_makefile_at = $(wildcard $(top_srcdir)/toc.$(PACKAGE_NAME).make.at)

TOC_HOME ?= /root/opennsl-man/deps/libs11n-1.2.10/toc/
# todo: check if this is under $(top_srcdir), so we can make this path relative.

##### deprecated:
toc_topdir ?= $(TOC_HOME)
toc_makesdir ?= $(TOC_HOME)/make
toc_bindir ?= $(TOC_HOME)/bin
# replacements are below...
#####

TOC_MAKESDIR = $(TOC_HOME)/make
TOC_BINDIR = $(TOC_HOME)/bin
TOC_SBINDIR = $(TOC_HOME)/sbin
TOC_PWD_FROM_TOPSRC = toc/tests
# e.g., in lib/foo, TOC_PWD_FROM_TOPSRC == lib/foo

TOC_MAKE = toc.make
TOP_TOC_MAKE = $(top_srcdir)/$(TOC_MAKE)



############################################################
# Experimental: auto-run configure if we think we need to
# Run configure with --enable-auto-reconfigure to get it.
ifeq (1,$(configure_enable_auto_reconfigure))
$(TOC_MAKE): $(TOC_MAKESDIR)/$(TOC_MAKE).at \
		$(toc_project_makefile_at) \
		$(top_srcdir)/configure $(top_srcdir)/configure.$(PACKAGE_NAME)
	@echo "$@ dependencies updated: [$?]. Reconfiguring..."; \
		touch $@; ${MAKE} toc-reconfigure
toc-reconfigure: $(TOC_MAKE)
	cd $(top_srcdir); ./configure $(CONFIGURE_ARGUMENTS) 

ifneq (,$(toc_config_vars))
$(toc_config_vars): $(TOC_MAKE)
endif
endif
# end autoreconfigure
############################################################3


##### some core utilities:
AWK_BIN = /usr/bin/awk
PERL_BIN = /usr/bin/perl
SED_BIN = /bin/sed
TAR_BIN = /bin/tar
GZIP_BIN = /bin/gzip
BZIP_BIN = /bin/bzip2
ZIP_BIN = 
INSTALLER_BIN = /root/opennsl-man/deps/libs11n-1.2.10/toc//bin/install-sh
TOC_MAKEDIST_BIN = /root/opennsl-man/deps/libs11n-1.2.10/toc//bin/makedist


DIST_FILES += Makefile

TOC_EMOTICON_OKAY=[1m:-)[m
TOC_EMOTICON_WARNING= [1m:-O[m
TOC_EMOTICON_ERROR=[1m:-([m






# TOC_SHARED_MAKEFILE is deprecated!
TOC_SHARED_MAKEFILE = $(wildcard $(top_srcdir)/@TOC_SHARED_MAKEFILE@)
ifneq (,$(TOC_SHARED_MAKEFILE))
    include $(TOC_SHARED_MAKEFILE)
endif

CLEAN_FILES += $(wildcard .toc.* core *~)
DISTCLEAN_FILES += $(wildcard \
			$(TOC_MAKE) toc.qmake \
			$(TOC_SHARED_MAKEFILE) toc_shared.qmake \
			toc.$(PACKAGE_NAME).make \
			toc.$(PACKAGE_NAME).configure.make \
			)

ifeq (.,$(top_srcdir))
    DISTCLEAN_FILES += toc.$(PACKAGE_NAME).configure.make
endif


##################################################### some common C/C++ stuff:
# C*FLAGS come from the gnu_cpp_tools toc test:
CONFIG_H = $(wildcard $(top_includesdir)/config.h)
INCLUDES += -I.

ifneq (,$(CONFIG_H))
    INCLUDES += -I$(top_srcdir)/include
    CPPFLAGS += -DHAVE_CONFIG_H=1
endif
#obsolete ifeq (1,$(configure_cpp_debug))
#    CPPFLAGS += -g
#endif
CPPFLAGS += $(INCLUDES)
##################################################### end C/C++ stuff



########################################################################
# configure_build_quietly IS GOING AWAY! DON'T USE IT
configure_build_quietly = 0

########################################################################
# CCDV_BIN: see TOC_HOME/bin/ccdv.c for details. This is a compiler
# front-end to pretty up C/C++ compiler output.
# We set this BEFORE INCLUDING PROJECT-SPECIFIC MAKEFILE CODE so 
# the ifeq()-based rules will behave properly.
CCDV_BIN = $(wildcard $(top_srcdir)/ccdv)
ifneq (,$(CCDV_BIN))
    CC := $(CCDV_BIN) $(CC)
    CXX := $(CCDV_BIN) $(CXX)
    AR := $(CCDV_BIN) $(AR)
    ifeq (.,$(top_srcdir))
        DISTCLEAN_FILES += $(CCDV_BIN)
    endif
endif
#  /CCDV_BIN
########################################################################


include $(TOC_MAKESDIR)/toc_functions.make
include $(TOC_MAKESDIR)/cleaner.make
include $(TOC_MAKESDIR)/subdirs_traverser.make
include $(TOC_MAKESDIR)/C_DEPS.make
include $(TOC_MAKESDIR)/INSTALL_XXX.make
include $(TOC_MAKESDIR)/dist.make



#### finally, get the project-specific code:
ifneq (,$(toc_project_makefile))
    include $(toc_project_makefile)
endif



