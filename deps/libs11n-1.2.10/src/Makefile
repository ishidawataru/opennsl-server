#!/usr/bin/make -f

include toc.make

SUBDIRS_SECOND = client
ifeq (1,$(s11n_CONFIG_ENABLE_PLUGINS))
      SUBDIRS_FIRST += plugin
      SUBDIRS_SECOND += unit
endif
SUBDIRS_FIRST += s11n io proxy lite nodeutil

SUBDIRS = $(SUBDIRS_FIRST) $(SUBDIRS_SECOND)
# s11n serializers lite sutil client
# sutil

DIST_FILES += $(wildcard in.*) test.cpp Makefile.for.generated.src.tree

SHARED_LIBS = libs11n 
libs11n_so_VERSION = $(PACKAGE_VERSION)
libs11n_so_LDADD = $(LIBS11N_LDADD) 
# not yet workie libs11n_so_SONAME = libs11n.so.200
libs11n_so_OBJECTS = \
	s11n/exception.o \
	s11n/s11n.o \
	s11n/s11n_node.o \
	lite/s11nlite.o	\
	io/data_node_io.o \
	io/strtool.o

ifeq (1,$(s11n_CONFIG_ENABLE_PLUGINS))
  libs11n_so_OBJECTS += plugin/plugin.o plugin/path_finder.o
endif

########################################################################
# Add only selected Serializers' object files. Unfortunately, this
# can't be done with a $(wildcard) because the wildcard is expanded as
# part of the SHARED_LIBS rules generation, which happens before the
# .o files exist.
ifneq (,$(s11n_BUILD_SERIALIZERS_LIST))
  ifneq (,$(filter compact,$(s11n_BUILD_SERIALIZERS_LIST)))
    libs11n_so_OBJECTS += io/compact/compact_serializer.o io/compact/compact.flex.o
  endif
  ifneq (,$(filter expat,$(s11n_BUILD_SERIALIZERS_LIST)))
    libs11n_so_OBJECTS += io/expat/expat_serializer.o
  endif
  ifneq (,$(filter funtxt,$(s11n_BUILD_SERIALIZERS_LIST)))
    libs11n_so_OBJECTS += io/funtxt/funtxt_serializer.o io/funtxt/funtxt.flex.o
  endif
  ifneq (,$(filter funxml,$(s11n_BUILD_SERIALIZERS_LIST)))
    libs11n_so_OBJECTS += io/funxml/funxml_serializer.o io/funxml/funxml.flex.o
  endif
  ifneq (,$(filter parens,$(s11n_BUILD_SERIALIZERS_LIST)))
    libs11n_so_OBJECTS += io/parens/parens_serializer.o io/parens/parens.flex.o
  endif
  ifneq (,$(filter simplexml,$(s11n_BUILD_SERIALIZERS_LIST)))
    libs11n_so_OBJECTS += io/simplexml/simplexml_serializer.o io/simplexml/simplexml.flex.o
  endif
  ifneq (,$(filter wesnoth,$(s11n_BUILD_SERIALIZERS_LIST)))
    libs11n_so_OBJECTS += io/wesnoth/wesnoth_serializer.o io/wesnoth/wesnoth.flex.o
  endif
# This would have been much simpler, of course:
#  libs11n_so_OBJECTS += $(wildcard $(patsubst %,io/%/*.o,$(s11n_BUILD_SERIALIZERS_LIST)))
endif
# end Serializer .o file additions
########################################################################

include $(TOC_MAKESDIR)/SHARED_LIBS.make

########################################################################
# Linking against a static s11n doesn't work because factory registrations
# done via the backwards approach we take are never triggered when linked
# this way :(
#STATIC_LIBS = libs11n
#libs11n_a_OBJECTS = $(libs11n_so_OBJECTS)
#include $(TOC_MAKESDIR)/STATIC_LIBS.make
########################################################################

BIN_PROGRAMS = test
test_bin_OBJECTS = test.o libs11n.so
test_bin_LDADD = $(LIBS11N_LDADD) -rdynamic
include $(TOC_MAKESDIR)/BIN_PROGRAMS.make
bins: bin-test


all: $(addprefix subdir-,$(SUBDIRS_FIRST)) SHARED_LIBS $(addprefix subdir-,$(SUBDIRS_SECOND))




