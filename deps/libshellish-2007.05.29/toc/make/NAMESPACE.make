#!/do/not/make
#
# Don't use this: use FILTER_FILES.make instead. It's more generic
# and easier to control/predict.
#
# A snippet to help filter source code files to replace a namespace.
# Usage:
# define:
#  NAMESPACE = mynamespace
#  NAMESPACE_TOKEN = token to replace when filtering. e.g. MY_NAMESPACE
#  NAMESPACE_PREFIX = filename prefix for pre-filtered files. Defaults to "ns.".
#  NAMESPACE_FILES = list of files matching $(NAMESPACE_PREFIX)xxxx, which will
#      be filtered out to xxxx.
#
# Then include this file.

NAMESPACE_PREFIX ?= ns.
NAMESPACE_TOKEN ?= NAMESPACE
ifeq (,$(NAMESPACE))
$(error You must set the variable NAMESPACE before including this file.)
endif

NAMESPACE_MAKE = $(TOC_MAKESDIR)/NAMESPACE.make
NAMESPACE_TMPFILE = .toc.NAMESPACE.tmp

NAMESPACE_FILTERED_FILES += $(patsubst $(NAMESPACE_PREFIX)%,%,$(NAMESPACE_FILES))
CLEAN_FILES += $(NAMESPACE_FILTERED_FILES)
# todo: use a makerules.NAMESPACE approach so deps can be set properly and
# we can avoid using the filenames as the targets (it currently causes rules
# conflicts with, e.g., C_DEPS).
NAMESPACE_TARGETS = $(NAMESPACE_FILTERED_FILES)
$(NAMESPACE_TARGETS): %: $(NAMESPACE_PREFIX)% Makefile $(top_srcdir)/toc.$(PACKAGE_NAME).make $(NAMESPACE_MAKE)
	@nsf=$<; \
		echo -ne "namespace $(NAMESPACE): "; \
		sed -e 's,$(NAMESPACE_TOKEN),$(NAMESPACE),g' < $$nsf > $(NAMESPACE_TMPFILE); \
		cmp -s $(NAMESPACE_TMPFILE) $@ && rm $(NAMESPACE_TMPFILE); \
		test -f $(NAMESPACE_TMPFILE) \
			&& { mv $(NAMESPACE_TMPFILE) $@; echo -n "[updated] "; } \
			|| echo -n "[up to date] "; \
		 echo $@

NAMESPACE: $(NAMESPACE_FILTERED_FILES)
#NAMESPACE:
#	@echo NAMESPACE_PREFIX=$(NAMESPACE_PREFIX)
#	@echo NAMESPACE_FILTERED_FILES=$(NAMESPACE_FILTERED_FILES)
#	@echo $(NAMESPACE_PREFIX)*
