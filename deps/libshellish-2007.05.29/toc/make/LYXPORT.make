#!/usr/bin/make -f
# A snippet for processing lyx files using lyxport.
# 
# usage:
#  LXYPORT_FILES = foo.lyx bar.lyx
#  include $(TOC_MAKESDIR)/LYXPORT.make
#
# Run the LYXPORT (or lyxport) target to process them. This support is
# fairly simplistic, and assumes:
#
# - lyxport will generate pdf/ps/html output in a subdirectory
#   named after the basename of each input file. This script
#   will REMOVE those subdirs after it is done, so BE CAREFUL
#   with your naming conventions!
#
# Implement a target named LYXPORT-post to do post-lyxport processing.
# e.g., for some files i like to rename them to have the project's
# version number:
#
#LYXPORT-post:
#	@echo "post-lyxport ..."; \
#	for l in $(LYXPORT_FILES); do \
#	for x in pdf ps html; do \
#		base=$${l%%.lyx}; \
#		cmd="mv $$base.$$x $$base-$(PACKAGE_VERSION).$$x"; \
#		echo $$cmd; $$cmd || exit; \
#	done; \
#	done


.PHONY: LYXPORT lyxport
LYXPORT_MAKE = $(TOC_MAKESDIR)/lyxport.make
LYXPORT_WARNING = $(LYXPORT_MAKE): this makefile requires LYXPORT_BIN. Try running the lyxport test.
ifeq (,$(LYXPORT_BIN))
LYXPORT: lyxport
lyxport:
	@echo "$(LYXPORT_WARNING)"
else

LYXPORT_L2HARGS ?= -nonavigation -show_section_numbers -split 0 -noimages
ifeq (,$(LYXPORT_ARGS))
    LYXPORT_ARGS = -c -tt
    LYXPORT_ARGS += --html --opts_l2h '$(LYXPORT_L2HARGS)'
    LYXPORT_ARGS += --ps --pdf
endif


LYXPORT_INCFILE = .toc.LYXPORT.make
CLEAN_FILES += $(LYXPORT_INCFILE)
$(LYXPORT_INCFILE): $(LYXPORT_RULES_GENERATOR) $(LYXPORT_MAKE) Makefile
ifeq (1,$(MAKING_CLEAN))
        @echo "$(MAKECMDGOALS): skipping LYXPORT rules generation."
else
	@echo "Generating LYXPORT rules."; \
	$(call toc_generate_rules,LYXPORT,$(LYXPORT_FILES)) > $@
endif
-include $(LYXPORT_INCFILE)

LYXPORT:
lyxport:


endif
# ^^^^ LYXPORT_BIN test
