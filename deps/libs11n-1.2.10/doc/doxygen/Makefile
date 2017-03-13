#!/usr/bin/make -f
# Makefile to build doxygen API docs for the collective libs11n libs.
#
# Requires the following bins in the $(PATH):
# doxygen, perl
# To build with 'dot' support (many more charts), do:
#   make clean
#   GOTDOT=1 make html

default: all

include toc.make
# find_bin: helper function to find $(1) in PATH
find_bin = $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(PATH)))))


DIST_FILES += Doxyfile.at index.txt


CLEAN_FILES += $(wildcard Doxyfile *~ html latex)

PERL_BIN ?= $(call find_bin,perl)
ifeq (,$(PERL_BIN))
    $(error Error: perl not found in PATH: $(PATH))
endif

DOXYGEN_BIN ?= $(call find_bin,doxygen)
ifeq (,$(DOXYGEN_BIN))
    $(error Error: doxygen not found in PATH: $(PATH))
endif




SINST_PREFIX = $(top_srcdir)
INCLUDES_HOME = $(top_srcdir)/include/s11n.net
INCLUDE_DIRS = $(top_srcdir)/include/s11n.net

DISTNAME = $(PACKAGE_NAME)-API-$(PACKAGE_VERSION)

# GOTDOT ?= $(shell which dot 2>/dev/null)
ifneq (,$(GOTDOT))
DISTNAME := $(DISTNAME)-with-charts
USE_DOT = YES
$(warning ********** WARNING: enabling 'dot' support. This might take a LONG time to build **********)
else
USE_DOT = NO
endif

Doxyfile: Doxyfile.at index.txt Makefile
	@echo "Creating $@..."; \
	sed -e 's,@PACKAGE_VERSION@,$(PACKAGE_VERSION),;' \
		-e 's,@DOXYGEN_INPUT@,index.txt $(INCLUDE_DIRS),;' \
		-e 's,@USE_DOT@,$(USE_DOT),;' \
		-e 's,@PERL@,$(PERL_BIN),;' \
	< $< > $@

html: Doxyfile
	@test -e $(top_srcdir)/include/s11n.net/s11n/s11n.hpp || { echo "You must first build s11n."; exit 127; }
	@echo "Building docs from headers under: $(INCLUDES_HOME)"
	$(DOXYGEN_BIN)

doxygen: html
	@fav=$$HOME/www/favicon.ico; test -f $$fav || exit 0; cp $$fav html/.

INSTALL_DEST = $(DESTDIR)$(prefix)/share/doc/libs11n/libs11n-$(PACKAGE_VERSION)
INSTALL_SUBDIR = API-doxygen
SYMLINK = libs11n-doxygen
INSTALL_ABS = $(INSTALL_DEST)/$(INSTALL_SUBDIR)
install: html
	test -d $(INSTALL_DEST) || mkdirhier $(INSTALL_DEST); \
	test -d $(INSTALL_DEST) || exit
	test -d $(INSTALL_ABS) && rm -fr $(INSTALL_ABS); \
	cp -r html $(INSTALL_ABS)
	cd $(INSTALL_ABS); cd ../; dname=$$(basename $$PWD); cd ..; \
		test -e $(SYMLINK) && rm $(SYMLINK); \
		ln -fs $$dname/$(INSTALL_SUBDIR) $(SYMLINK)


tar: html
	mv html $(DISTNAME)
	tar czf $(DISTNAME).tar.gz $(DISTNAME)
	tar cjf $(DISTNAME).tar.bz2 $(DISTNAME)
	rm -fr $(DISTNAME)

all:
	@echo "Run 'make doxygen' to build the doxygen docs."


dist:
	mkdir $(DISTNAME)
	cp $(DIST_FILES) $(DISTNAME)
	tar czf $(DISTNAME).tar.gz $(DISTNAME)
	tar cjf $(DISTNAME).tar.bz2 $(DISTNAME)
	rm -fr $(DISTNAME)

