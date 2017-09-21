include toc.make

SOURCES = \
	exception.cpp \
	s11n.cpp \
	s11n_node.cpp

HEADERS = \
	abstract_creator.hpp \
	algo.hpp \
	algo.tpp \
	classload.hpp \
	classload.tpp \
	exception.hpp \
	export.hpp \
	factory.hpp \
	factory_reg.hpp \
	functional.hpp \
	phoenix.hpp \
	reg_s11n_traits.hpp \
	s11n_debuggering_macros.hpp \
	s11n.hpp \
	s11n_node.hpp \
	serialize.hpp \
	serialize.tpp \
	tags.hpp \
	traits.hpp \
	type_traits.hpp \
	variant.hpp


OBJECTS = $(addsuffix .o,\
	exception \
	s11n \
	s11n_node \
	)

objects: $(OBJECTS)

INSTALL_PACKAGE_HEADERS = $(HEADERS) s11n_config.hpp
HEADERS_RELPATH = include/s11n.net/s11n
INSTALL_PACKAGE_HEADERS_DEST = $(prefix)/$(HEADERS_RELPATH)

DIST_FILES += $(wildcard *.at) $(SOURCES) $(HEADERS) 

DISTCLEAN_FILES += s11n_config.hpp

SYMLINK_HEADERS = $(INSTALL_PACKAGE_HEADERS)
SYMLINK_HEADERS_DEST = $(top_srcdir)/$(HEADERS_RELPATH)
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make

$(OBJECTS): SYMLINK_HEADERS subdirs
all: $(OBJECTS)
