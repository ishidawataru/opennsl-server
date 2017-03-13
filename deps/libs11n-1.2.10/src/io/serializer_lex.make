#!/do/not/make
# Intented to be included by lex-based Serializer Makefiles.
# See the included serializers subdirs for sample usage.

########################################################################
# Requires:
#
#   SERILIZER_BASE = shortname

#   e.g.: compact, parens, funtxt
#
#   Files:
#      $(SERIALIZER_BASE)_serializer.{c,h}pp
#      $(SERIALIZER_BASE).flex
#
#   In order to provide dynamic file handler lookup, client lexers must
#   register their FlexLexer concrete subclass with with the FlexLexer
#   classloader, as demonstrated in the included lexers.
########################################################################

ifeq (,$(SERIALIZER_BASE))
  $(error SERIALIZER_BASE must be set to the "short" form of the Serializer name before including this file.)
endif

SERIALIZER = $(SERIALIZER_BASE)_serializer

SERIALIZER_FLEX = $(SERIALIZER_BASE).flex
# SERIALIZER_FLEX_CPP might be locally generated or might come with the
# dist tarball. We can only rely on flex 2.5.4, so we ship
# the lexers generated on the dev machine with the tarball.
SERIALIZER_FLEX_CPP = $(SERIALIZER_FLEX).cpp

SERIALIZER_LEXER_PREFIX = $(SERIALIZER_BASE)_data_node
SERIALIZER_LEXER_CLASS = $(SERIALIZER_LEXER_PREFIX)FlexLexer
SERIALIZER_FlexLexer_hpp = $(SERIALIZER_LEXER_CLASS).hpp

SOURCES = $(SERIALIZER).cpp
HEADERS = $(SERIALIZER).hpp

DIST_FILES += $(SOURCES) $(HEADERS) \
	$(SERIALIZER_FLEX) $(SERIALIZER_FLEX_CPP) \
	$(SERIALIZER_FlexLexer_hpp)


LexerTemplate_hpp = ../LexerTemplate.hpp
DISTCLEAN_FILES += $(SERIALIZER_FlexLexer_hpp)
$(SERIALIZER_FlexLexer_hpp): $(LexerTemplate_hpp) Makefile
	sed -e 's/yyFlexLexer/$(SERIALIZER_LEXER_CLASS)/g' $(LexerTemplate_hpp) > $@

########################################################################
# headers stuff...
IOINCLUDES_PATH = include/s11n.net/s11n/io
INSTALL_PACKAGE_HEADERS_DEST = $(prefix)/$(IOINCLUDES_PATH)
# $(SERIALIZER_FlexLexer_hpp) is only installed so that we can
# easily make a copy for a build on platforms with no flex, like MS Windows.
INSTALL_PACKAGE_HEADERS = $(HEADERS) $(SERIALIZER_FlexLexer_hpp)
SYMLINK_HEADERS = $(INSTALL_PACKAGE_HEADERS)
SYMLINK_HEADERS_DEST = $(top_srcdir)/$(IOINCLUDES_PATH)
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make
########################################################################

OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES) $(SERIALIZER_FLEX_CPP))

########################################################################
# if we are using a local flex to build *.flex, the following block
# becomes active. Only flex 2.5.4[a] are known to generate working
# code for the serializers, thus we ship pre-flexed copies in the
# code distribution for use on systems where we don't trust flex.
ifneq (,$(FLEX_BIN))
ifeq (1,$(SERIALIZERS_USE_LOCAL_FLEX))

########################################################################
# THIS IS A HORRIBLE KLUDGE to work around to some inexplicable build bug.
# See the comments below labeled 'WTF' to understand the [non]sense of this.
$(shell test -f $(SERIALIZER_FLEX_CPP) -a $(SERIALIZER_FLEX) -nt $(SERIALIZER_FLEX_CPP) && rm $(SERIALIZER_FLEX_CPP))
########################################################################

FlexLexer_h = s11n.net/s11n/io/FlexLexer.h
FlexLexer_hpp = s11n.net/s11n/io/FlexLexer.hpp

DISTCLEAN_FILES += $(SERIALIZER_FLEX_CPP)
SERIALIZER_FLEX_FLAGS = -p -+ -B -P$(SERIALIZER_LEXER_PREFIX)


########################################################################
# WTF!?!?! When i add SERIALIZER_FLEX as a dep for the
# SERIALIZER_FLEX_CPP target i get warnings about circular deps being
# dropped and it tries to compile $(FlexLexer_hpp)!!!
# AND it DELETES $(SERIALIZER_FLEX)!!!!! Aaaarrrgggg!
$(SERIALIZER_FLEX_CPP): $(SERIALIZER_FlexLexer_hpp) Makefile ../serializer_lex.make
	$(FLEX_BIN) $(SERIALIZER_FLEX_FLAGS) -t $(SERIALIZER_FLEX)  > $@
	@echo -n "Patching $@ for recent C++ standards and $(FlexLexer_hpp)... "; \
		perl -i -p \
		-e 's|<FlexLexer\.h>|<$(FlexLexer_hpp)>|g;' \
		-e 's|class (std\::)?istream;|#include <iostream>|;' \
		-e 's/\biostream\.h\b/iostream/;' \
		-e 's|^\s*//(.*isatty.*)|$1|;' \
		-e 'next if m/\biostream\b/;' \
		-e 'next if m/::[io]stream/;' \
		-e 's/(\bostream\b|\bistream\b)([^\.])/std::$$1$$2/g;' $@ || exit ; \
		perl -i -ne 'next if $$_ =~ m|unistd\.h|; print;' $@ || exit; \
		echo
# ^^^ reminder: we remove unistd.h so that the files will build under Windows (hopefully)

endif
endif
# ^^^^ SERIALIZERS_USE_LOCAL_FLEX
########################################################################


#SHARED_LIBS = $(SERIALIZER)
#$(SERIALIZER)_so_INSTALL = 0
# INSTALL_LIBEXECS_DEST = $(prefix)/lib/s11n
# $(SERIALIZER)_so_OBJECTS = $(OBJECTS)
# $(SERIALIZER)_so_VERSION = $(PACKAGE_VERSION)
# include $(TOC_MAKESDIR)/SHARED_LIBS.make

serializer: $(SERIALIZER_FlexLexer_hpp) SYMLINK_HEADERS $(OBJECTS)
# $($(SERIALIZER)_so_OBJECTS)
# SHARED_LIBS
