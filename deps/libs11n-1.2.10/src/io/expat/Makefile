INSTALL_LIBEXECS_DEST = $(prefix)/lib/s11n

include toc.make



HEADERS = \
	expat_serializer.hpp

SOURCES = expat_serializer.cpp

INSTALL_PACKAGE_HEADERS = $(HEADERS)
INSTALL_PACKAGE_HEADERS_DEST = $(prefix)/include/s11n.net/s11n/io
DIST_FILES += $(HEADERS) $(SOURCES)


SYMLINK_HEADERS = $(INSTALL_PACKAGE_HEADERS)
SYMLINK_HEADERS_DEST = $(top_srcdir)/include/s11n.net/s11n/io
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make

#SHARED_LIBS = expat_serializer
#INSTALL_LIBEXECS_DEST = $(prefix)/lib/s11n
#expat_serializer_so_OBJECTS = expat_serializer.o
#expat_serializer_so_LDADD = -lexpat
#expat_serializer_so_VERSION = $(PACKAGE_VERSION)
#include $(TOC_MAKESDIR)/SHARED_LIBS.make

expat_serializer.o: SYMLINK_HEADERS
all: expat_serializer.o
# $(expat_serializer_so_OBJECTS)
# SHARED_LIBS
