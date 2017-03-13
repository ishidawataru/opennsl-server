#!make
# usage:
#  set:
#          C_BINS_FROM_SOURCES = foo bar
#          foo_bin_SOURCES = foo.c foo2.c
#          bar_bin_SOURCES = bar.c bar2.c foobar.c
# optional: foo_bin_OBJECTS = somethingdifferent.o
# optional: foo_bin_CFLAGS = -DCOMPILING_FOO=1 -I/somewhere/different
# optional: foo_bin_LFLAGS = -lfl -lfoo
#
# all: bins
#
# This compiles, all in one go, 'bar' from $(bar_bin_SOURCES)


C_BINS_FROM_SOURCES_MAKEFILE = $(TOC_MAKESDIR)/C_BINS_FROM_SOURCES.make


ifneq (,$(C_BINS_FROM_SOURCES))
C_BINS_FROM_SOURCES_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.C_BINS_FROM_SOURCES
C_BINS_FROM_SOURCES_COMMON_DEPS += Makefile $(C_BINS_FROM_SOURCES_MAKEFILE)
C_BINS_FROM_SOURCES_DEPSFILE = .toc.C_BINS_FROM_SOURCES.make
CLEAN_FILES += $(C_BINS_FROM_SOURCES_DEPSFILE)
$(C_BINS_FROM_SOURCES_DEPSFILE): $(C_BINS_FROM_SOURCES_COMMON_DEPS) $(C_BINS_FROM_SOURCES_RULES_GENERATOR)
ifneq (,$(strip $(filter distclean clean,$(MAKECMDGOALS))))
	@echo "$(MAKECMDGOALS): skipping C_BINS_FROM_SOURCES rules generation."
else
	@echo "Generating C_BINS_FROM_SOURCES rules."; \
	$(call toc_generate_rules,C_BINS_FROM_SOURCES,$(C_BINS_FROM_SOURCES)) > $@
endif
# ^^^ making clean?

-include $(C_BINS_FROM_SOURCES_DEPSFILE)

deps: $(C_BINS_FROM_SOURCES_DEPSFILE)

endif
# ^^^ got $(C_BINS_FROM_SOURCES)?
