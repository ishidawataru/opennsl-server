#!/do/not/make
# A toc snippet to ... todo
#
# Usage:
#
# COPY_SETS = set1 set2 [...]
# # optional: COPY_SETS_DEST = /a/path
# set1_COPY_DEST = /some/path # defaults to $(COPY_SETS_DEST)
# set1_COPY_SOURCES = myfile1.cpp myfile2.cpp
# set2_COPY_DEST = /another/path
# set2_COPY_SOURCES = myfile3.cpp myfile4.cpp
# include $(TOC_MAKESDIR)/COPY_SETS.make
#
# It creates targets:
#
#  COPY_SETS
#  copy-[set name]
#
# Those targets simply copy the given sources to their defined
# destinations.
#
#########################################################################################

COPY_SETS_MAKEFILE = $(TOC_MAKESDIR)/COPY_SETS.make
ifeq (,$(COPY_SETS))
$(error $(COPY_SETS_MAKEFILE): you must define COPY_SETS, plus some other vars, before including this file! Read the docs in this file)
endif

COPY_SETS_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.COPY_SETS
$(COPY_SETS_RULES_GENERATOR):
$(COPY_SETS_MAKEFILE):

COPY_SETS: FORCE

COPY_SETS_INCFILE = .toc.COPY_SETS.make
CLEAN_FILES += $(COPY_SETS_INCFILE)
$(COPY_SETS_INCFILE): Makefile $(COPY_SETS_RULES_GENERATOR) $(COPY_SETS_MAKEFILE)
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping COPY_SETS rules generation."
else
	@echo "Generating COPY_SETS rules."; \
	$(call toc_generate_rules,COPY_SETS,$(COPY_SETS)) > $@
endif
-include $(COPY_SETS_INCFILE)

