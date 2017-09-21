#!/do/not/make
# A toc snippet to provide generic file filtering support. (Originally
# written to filter a namespace name in some C++ code.)
# Depends on the script makerules.FILE_FILTERS, plus some toc-related
# conventions.
#
# stephan@s11n.net, 1 Dec 2003
#
# Usage:
#
# Define:
#  FILE_FILTERS = filterX filterY
#      These are logical names for filter sets, and should be unique with a Makefile,
#      especially, there should be no targets with the names in $(FILE_FILTERS).
#
#  either:
#      FILE_FILTERS_BIN = /path/to/filter/app + args
#  or:
#      filterX_FILTER_BIN = /path/to/filter/app + args (defaults to 'perl -p')
#
#  filterX_FILTER_RULES = -e 's|rules for your filter|...|' (FILTER_BIN must accept this
#          as a command-line argument)
#
#  filterX_FILTER_SOURCES = list of input files, all of which must have some common prefix.
#
#  filterX_FILTER_NAMESED = sed code to translate x_FILTER_SOURCES to target filenames. Gets
#          applied to each name in x_FILTER_SOURCES. If this uses the $ pattern you must
#          use $$ instead, e.g. s/$$/.out/. BE CAREFUL!
#
#  filterX_FILTER_DEPS = optional list of extra deps. Automatically added are 
#          all input/control files used in creating the rules, including Makefile.
#
#
#  Generated files:
#  - are named .toc.FILE_FILTERS.*
#  - are added to $(CLEAN_FILES)
#  - are not changed unless out-of-date, so they are dependencies-safe.
#  - get their own target names. This may cause collisions with other targets,
#      but presumably only one target is responsible for creating any given
#      file.
#
#
#########################################################################################
# BUG WARNING:  BUG WARNING:  BUG WARNING:  BUG WARNING:  BUG WARNING:  BUG WARNING:
#
# It works by creating intermediary makefiles (.toc.FILE_FILTERS.*), so:
#
# When you remove/rename entries from FILE_FILTERS (as your project changes) you
# will need to 'rm .toc.FILE_FILTERS.*' in order to be able to run make again, as some
# pre-generated rules may become invalidated and generate now-bogus errors which will
# kill make before it can run any targets (e.g., clean).
#
#########################################################################################
# Sample usage:
#  FILE_FILTERS = namespace filter2
#  # optional: namespace_FILTER_BIN = $(PERL_BIN) -p
#  namespace_FILTER_RULES = -e 's|PACKAGE_NAMESPACE|$(PACKAGE_NAMESPACE)|g'
#  namespace_FILTER_SOURCES = $(wildcard src/*.cpp src/*.h)
#  namespace_FILTER_NAMESED = s,src/,,
#  ... similar for filter2_FILTER_XXX
#  include $(TOC_MAKESDIR)/FILE_FILTERS.make
#
#  That will filter src/*.{cpp,h} to *.{cpp,h}, replacing PACKAGE_NAMESPACE
#  with $(PACKAGE_NAMESPACE)
#
# To process it, either:
#    all: FILE_FILTERS mytarget othertarget
#  or:
#    all: filter-namespace mytarget filter-filter2 othertarget
#  or:
#    all: namespace mytarget filter2 othertarget
#
#########################################################################################

FILE_FILTERS_MAKEFILE = $(TOC_MAKESDIR)/FILE_FILTERS.make
ifeq (,$(FILE_FILTERS))
$(error $(FILE_FILTERS_MAKEFILE): you must define FILE_FILTERS, plus some other vars, before including this file! Read the docs in this file)
endif

FILE_FILTERS_BIN ?= $(PERL_BIN) -p

FILE_FILTERS_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.FILE_FILTERS
$(FILE_FILTERS_RULES_GENERATOR):
$(FILE_FILTERS_MAKEFILE):

FILE_FILTERS_INCFILE = .toc.FILE_FILTERS.make
CLEAN_FILES += $(FILE_FILTERS_INCFILE)
$(FILE_FILTERS_INCFILE): Makefile $(FILE_FILTERS_RULES_GENERATOR) $(FILE_FILTERS_MAKEFILE)
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping FILE_FILTERS rules generation."
else
	@echo "Generating FILE_FILTERS rules."; \
	$(call toc_generate_rules,FILE_FILTERS,$(FILE_FILTERS)) > $@
endif
-include $(FILE_FILTERS_INCFILE)

FILE_FILTERS:
