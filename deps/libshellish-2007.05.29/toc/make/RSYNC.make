#!/do/not/make
#
# a toc snippet to help provide support for uploading stuff over rsync.
# It expects $(RSYNC_BIN) to be set via the configure process.
#
# Author: stephan - sgbeal@users.sourceforge.net
#
# Usage:
#
# Define:
#
# RSYNC_TARGETS = target1 [...targetN]
# optional: RSYNC_FLAGS = flags to pass to scp, like -C to enable compression
# optional: RSYNC_DEST = [user@remote:]/destination/path
# optional: RSYNC_CHDIR = [path to cd to before starting]
#
# For each target in SCP_TARGETS you must define the following:
#
# target1_RSYNC_FILES = list of files/dirs to rsync
# optional: target1_RSYNC_DEST = [user@remote:]/destination/path
# optional: target1_RSYNC_FLAGS = takes precedence over RSYNC_FLAGS
# optional: target1_RSYNC_CHDIR = [path to cd to before starting]
#
# xxx_RSYNC_CHDIR is an odd-case var, and is not required (it defaults
# to .)
#
# At least one of RSYNC_DEST or target_RSYNC_DEST must be set,
# and the target_RSYNC_DEST takes precedence.
#
# To run all RSYNC targets run one of:
#   make RSYNC
#   make rsync
#
# To run one of them:
#   make RSYNC-targetname
#   make rsync-targetname
#

RSYNC_MAKEFILE = $(TOC_MAKESDIR)/RSYNC.make
ifeq (,$(RSYNC_BIN))
$(warning you must define RSYNC_BIN before using the RSYNC targets. Try running the rsync toc test.)
RSYNC:
$(patsubst %,rsync-%,$(RSYNC_TARGETS)):
$(patsubst %,RSYNC-%,$(RSYNC_TARGETS)):
else


RSYNC_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.RSYNC
$(RSYNC_RULES_GENERATOR):
$(RSYNC_MAKEFILE):

RSYNC_INCFILE = .toc.RSYNC.make
CLEAN_FILES += $(RSYNC_INCFILE)
$(RSYNC_INCFILE): $(RSYNC_RULES_GENERATOR) $(RSYNC_MAKEFILE) Makefile
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping RSYNC rules generation."
else
	@echo "Generating RSYNC rules."; \
	$(call toc_generate_rules,RSYNC,$(RSYNC_TARGETS)) > $@
endif
-include $(RSYNC_INCFILE)

.PHONY: RSYNC
RSYNC:
rsync:

endif
# ^^^ end no-rsync guard
