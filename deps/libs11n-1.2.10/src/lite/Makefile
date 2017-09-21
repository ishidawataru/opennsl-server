include toc.make

S11N_NS ?= s11n

SOURCES = s11nlite.cpp


HEADERS = client_api.hpp \
	micro_api.hpp \
	s11nlite.hpp \
	simple_config.hpp

OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
objects: $(OBJECTS)

INSTALL_PACKAGE_HEADERS = $(HEADERS)


# DIST_FILES += $(sort $(wildcard *.cpp) $(wildcard *.h) $(wildcard *.make))
DIST_FILES += $(wildcard *.at) $(SOURCES) $(HEADERS)

SYMLINK_HEADERS = $(INSTALL_PACKAGE_HEADERS)
SYMLINK_HEADERS_DEST = $(top_srcdir)/include/s11n.net/s11n
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make

$(OBJECTS): SYMLINK_HEADERS
all: $(OBJECTS)

