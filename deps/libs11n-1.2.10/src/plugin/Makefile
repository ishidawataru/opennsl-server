#!/usr/bin/make -f

include toc.make

SOURCES = \
	path_finder.cpp \
	plugin.cpp

HEADERS = \
	path_finder.hpp \
	plugin.hpp \
	plugin_config.hpp

DIST_FILES += \
	$(SOURCES) \
	path_finder.hpp \
	plugin.hpp \
	plugin.dl.cpp \
	plugin.win32.cpp \
	plugin.noop.cpp \
	plugin_config.hpp.at

DISTCLEAN_FILES += plugin_config.hpp

OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))

HEADERS_RELPATH = include/s11n.net/s11n/plugin

INSTALL_PACKAGE_HEADERS = $(HEADERS)
INSTALL_PACKAGE_HEADERS_DEST = $(prefix)/$(HEADERS_RELPATH)

SYMLINK_HEADERS = $(INSTALL_PACKAGE_HEADERS)
SYMLINK_HEADERS_DEST = $(top_srcdir)/$(HEADERS_RELPATH)
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make


#WEIRD: including this causes deps to be generated multiple times
# and the copy rules refuse to actually trigger.
#COPY_SETS = sources
# headers
#sources_COPY_SOURCES = $(SOURCES) $(wildcard plugin.*.cpp)
#sources_COPY_DEST = $(S11N_PLAIN_SOURCES_ROOT)/src
#headers_COPY_DEST = $(S11N_PLAIN_SOURCES_ROOT)/$(HEADERS_RELPATH)
#headers_COPY_SOURCES = $(HEADERS)
#include $(TOC_MAKESDIR)/COPY_SETS.make
#COPY_SETS:
#copy: copyset-sources COPY_SETS
#	#$(sources_COPY_DEST)
#	#$(sources_COPY_SOURCES)

$(OBJECTS): SYMLINK_HEADERS
all: $(OBJECTS)
