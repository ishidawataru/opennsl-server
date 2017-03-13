#!/do/not/make
#
# a toc snippet to help provide support for uploading stuff over scp
# It expects $(SCP_BIN) to be set via the configure process.
#
# Author: stephan - sgbeal@users.sourceforge.net
#
# Usage:
#
# Define:
#
# SCP_TARGETS = target1 [...targetN]
# optional: SCP_FLAGS = flags to pass to scp, like -C to enable compression
# optional: SCP_LOGIN = user@host
# optional: SCP_PATH = /destination/path/on/remote
#
# For each target in SCP_TARGETS you must define the following:
# target1_SCP_FILES = list of files
# optional: target1_SCP_LOGIN = user@host
# optional: target1_SCP_PATH = /destination/path/on/remote
#
# At least one of SCP_{LOGIN,PATH} or target_{LOGIN,PATH} must be set,
# and the target_xxx takes precedence.
#
# To run all SCP targets:
#   make SCP
# To run one of them:
#   make scp-targetname
# or
#   make SCP-targetname
# (they're the same)
#
# When adding/removing SCP_TARGETS you may need to 'rm .toc.SCP.make'
# so some generated rules don't cause Make to break before, e.g.,
# the rules can be recreated.
#
# Tip: it is sometimes useful to do this before including
# SCP.make:
#   SCP_BIN := echo $(SCP_BIN)
#

SCP_MAKEFILE = $(TOC_MAKESDIR)/SCP.make
ifeq (,$(SCP_BIN))
$(warning you must define SCP_BIN before using the SCP targets. Try running the ssh_tools toc test.)
SCP:
$(patsubst %,scp-%,$(SCP_TARGETS)):
else


SCP_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.SCP
$(SCP_RULES_GENERATOR):
$(SCP_MAKEFILE):

SCP_INCFILE = .toc.SCP.make
CLEAN_FILES += $(SCP_INCFILE)
$(SCP_INCFILE): $(SCP_RULES_GENERATOR) $(SCP_MAKEFILE) Makefile
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping SCP rules generation."
else
	@echo "Generating SCP rules."; \
	$(call toc_generate_rules,SCP,$(SCP_TARGETS)) > $@
endif
-include $(SCP_INCFILE)

.PHONY: SCP
SCP:

endif
# ^^^ end no-scp guard
