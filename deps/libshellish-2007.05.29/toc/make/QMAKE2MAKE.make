#!/usr/bin/make -f
########################################################################
# Generates make.X for X.qmake, where X.qmake is expected to
# be a qmake-compatible makefile.
#
# Why? So you can have multiple qmake files in one dir, something
# which qmake doesn't inherently allow for. Since qmake only allows
# for one significant target in each makefile, this helper gets
# around forcing client-side trees to restructure when using qmake.
#
# Sample usage:
#
#   default: all
#   QMAKE2MAKE = foo bar # requires that {foo,bar}.qmake exist
#   include path/to/QMAKE2MAKE.make
#   all: QMAKE2MAKE
#
# It generates and runs make.X for each X.qmake, where X is an entry
# in $(QMAKE2MAKE). It also provides these global targets:
#  qmake-X-{clean,distclean,install,uninstall} qmake-X
#
# It also modifies the deps for clean, distclean, install and uninstall
# to call the appropriate targets in the qmake-processed files.
#
# Note that qmake normally uses a .pro extension for qmake files, but
# i find this to be a poor choice, giving the long usage history of
# that extension for so many file formats.
#
# ----- stephan@s11n.net
########################################################################

QMAKE2MAKE_MAKEFILE = $(TOC_MAKESDIR)/QMAKE2MAKE.make
ifeq (,$(wildcard $(QTDIR)))
$(error $(QMAKE2MAKE_MAKEFILE) requires the QTDIR var. Maybe you need to run the qt configure test?)
endif

ifeq (,$(QMAKE2MAKE))
$(error $(QMAKE2MAKE_MAKEFILE) requires the var QMAKE2MAKE to be set to the base name of at least one existing *.qmake file.)
endif

QMAKE2MAKE_GENERATOR = $(TOC_MAKESDIR)/makerules.QMAKE2MAKE
QMAKE2MAKE_DEPSFILE = .toc.QMAKE2MAKE.make
$(QMAKE2MAKE_DEPSFILE): $(QMAKE2MAKE_GENERATOR) $(QMAKE2MAKE_MAKEFILE) Makefile
	@echo "Generating QMAKE2MAKE rules."; \
	$(QMAKE2MAKE_GENERATOR) $(QMAKE2MAKE) > $@
-include $(QMAKE2MAKE_DEPSFILE)

QMAKE2MAKE: qmake-all

deps: $(QMAKE2MAKE_DEPSFILE)

