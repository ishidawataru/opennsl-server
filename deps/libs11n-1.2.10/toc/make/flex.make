#!make
# snippet to compile .flex files into .cpp
#
# usage:
# define:
#    FLEXES = foo bar
#    FLEXES_ARGS = optional args to pass to flex.
#    foo_FLEXES_ARGS = optional args for foo.flex
#
# all: flexes
#
# You must have foo.flex for each $(FLEXES) and foo.flex.cpp will
# be generated. It will be "massaged" a bit to allow it to compile
# under newer GCC versions (3.x).
#

FLEX_CPPFILE_SUFFIX ?= .cpp
FLEX_FILE_SUFFIX ?= .flex

%$(FLEX_FILE_SUFFIX):# undefine make's implicit rule

FLEX_MAKEFILE = $(TOC_MAKESDIR)/flex.make
ifneq (,$(FLEXES))

FLEX_BIN = $(firstword $(wildcard $(addsuffix /flex,$(subst :, ,$(PATH)))))
ifeq (,$(FLEX_BIN))
$(error This makefile snippet requires that 'flex' be found in the PATH!)
endif


FLEX_FLEX_FILES = $(addsuffix $(FLEX_FILE_SUFFIX),$(FLEXES))
FLEX_CPP_FILES = $(addsuffix $(FLEX_CPPFILE_SUFFIX),$(FLEX_FLEX_FILES))

# $(FLEX_FLEX_FILES): Makefile
# %.flex: Makefile
%$(FLEX_FILE_SUFFIX)$(FLEX_CPPFILE_SUFFIX): %$(FLEX_FILE_SUFFIX) $(FLEX_MAKEFILE)
	@cmd='$(FLEX_BIN) $(FLEXES_ARGS) $($(basename $(patsubst %$(FLEX_FILESUFFIX)$(FLEX_CPPFILE_SUFFIX),%,$@))_FLEXES_ARGS) -t $< '; \
	echo $$cmd '> $@'; \
	$$cmd > $@ || { err=$$?; exit $$err; }; \
		perl -i -p \
		-e 's|class std\::istream;|#include <iostream>|;' \
		-e 's/\biostream\.h\b/iostream/;' \
		-e 'next if m/\biostream\b/;' \
		-e 'next if m/::[io]stream/;' \
		-e 's/(\bostream\b|\bistream\b)([^\.])/std::$$1$$2/g;' $@

#	@echo "Flexing $< to $@, plus doing iostream hacks :/.";

CLEAN_FILES += $(FLEX_CPP_FILES)
# $(FLEX_FLEX_FILES)

flexes: $(FLEX_CPP_FILES)
FLEXES: flexes
else
flexes:
endif
