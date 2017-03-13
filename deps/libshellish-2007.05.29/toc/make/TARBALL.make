#!/do/not/make
#
# a toc snippet to help provide support for creating tarballs.
# It expects $(TAR_BIN) to be set via the configure process (e.g., via the
# gnu_tar test).
#
# Define:
#
# TARBALL_TARGETS = target1 [...targetN]
# optional: TARBALL_FLAGS = flags to pass to tar (e.g., -z for gzip)
# optional: TARBALL_CHDIR = [path to cd to before starting]
#
# For each target in TARBALL_TARGETS you must define the following:
#
# target1_TARBALL_FILES = list of files/dirs to tar
# target1_TARBALL = destination filename
# optional: target1_TARBALL_FLAGS = takes precedence over TARBALL_FLAGS
# optional: target1_TARBALL_CHDIR = [path to cd to before starting]
#
# xxx_TARBALL_CHDIR is an odd-case var, and is not required (it defaults
# to .)
#
# To run all TARBALL targets run one of:
#   make TARBALL
#
# To run one of them:
#   make TARBALL-targetname

TARBALL_MAKEFILE = $(TOC_MAKESDIR)/TARBALL.make
ifeq (,$(TAR_BIN))
$(warning you must define TAR_BIN before using the TARBALL targets. Try running the gnu_tar toc test.)
TARBALL:
$(patsubst %,tar-%,$(TARBALL_TARGETS)):
else


TARBALL_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.TARBALL
$(TARBALL_RULES_GENERATOR):
$(TARBALL_MAKEFILE):

TARBALL_INCFILE = .toc.TARBALL.make
CLEAN_FILES += $(TARBALL_INCFILE)
$(TARBALL_INCFILE): $(TARBALL_RULES_GENERATOR) $(TARBALL_MAKEFILE) Makefile
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping TARBALL rules generation."
else
	@echo "Generating TARBALL rules."; \
	$(call toc_generate_rules,TARBALL,$(TARBALL_TARGETS)) > $@
endif
-include $(TARBALL_INCFILE)

.PHONY: TARBALL
TARBALL:

endif
# ^^^ end no-tar guard
