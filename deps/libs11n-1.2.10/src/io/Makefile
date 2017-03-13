include toc.make

SUBDIRS = $(s11n_BUILD_SERIALIZERS_LIST)
# compact funtxt funxml parens simplexml wesnoth

#ifeq (1,$(HAVE_LIBEXPAT))
#    SUBDIRS += expat
#endif

SOURCES = \
	data_node_io.cpp \
	strtool.cpp

# t10n.cpp

HEADERS = \
	data_node_format.hpp \
	data_node_io.hpp \
	FlexLexer.hpp \
	reg_serializer.hpp \
	serializers.hpp \
	strtool.hpp

HEADERS_NOINST = \
	LexerTemplate.hpp

OBJECTS = data_node_io.o strtool.o
# t10n.o

IOINCLUDES_PATH = include/s11n.net/s11n/io
INSTALL_PACKAGE_HEADERS = $(HEADERS)
INSTALL_PACKAGE_HEADERS_DEST = $(prefix)/$(IOINCLUDES_PATH)

DIST_FILES += $(SOURCES) $(HEADERS) $(HEADERS_NOINST) \
	serializer_lex.make


SYMLINK_HEADERS = $(INSTALL_PACKAGE_HEADERS)
SYMLINK_HEADERS_DEST = $(top_srcdir)/$(IOINCLUDES_PATH)
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make

INSTALL_LIBEXECS_DEST = $(prefix)/lib/s11n
$(OBJECTS): SYMLINK_HEADERS
all: $(OBJECTS) subdirs
