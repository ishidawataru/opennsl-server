include toc.make

SOURCES = Readline.cpp

HEADERS = Readline.hpp 

ifeq (1,$(HAVE_LIBS11N))
    HEADERS += Readline_s11n.hpp
endif

DISTCLEAN_FILES = Readline_config.hpp


DIST_FILES += Readline.cpp Readline.hpp Readline_s11n.hpp \
	test.cpp \
	Readline_config.hpp.at

INSTALL_HEADERS = $(HEADERS) Readline_config.hpp


LIBNAME = libreadline_cpp
SHARED_LIBS = $(LIBNAME)
$(LIBNAME)_so_OBJECTS = Readline.o
$(LIBNAME)_so_VERSION = $(PACKAGE_VERSION)
SHARED_LIBS_LDADD = $(LIBREADLINECPP_LDADD)
# $(LIBREADLINECPP_LDADD)
STATIC_LIBS = $(SHARED_LIBS)
$(LIBNAME)_a_OBJECTS = $($(LIBNAME)_so_OBJECTS)

include $(TOC_MAKESDIR)/SHARED_LIBS.make
include $(TOC_MAKESDIR)/STATIC_LIBS.make

BIN_PROGRAMS = test
test_bin_OBJECTS = test.o
test_bin_LDADD =  -L. -lreadline_cpp
include $(TOC_MAKESDIR)/BIN_PROGRAMS.make
INSTALL_BINS =


all: SHARED_LIBS STATIC_LIBS
