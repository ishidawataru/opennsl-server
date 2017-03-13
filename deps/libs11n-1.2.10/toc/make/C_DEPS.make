#!/do/not/make
# makefile snippet
# Usage:
#  include path/to/this/file
#
# That will set up a 'deps' target which deps all C/C++ files and updates
# the deps when any of those files, or the Makefile, changes.
# That's normally all there is to it.
#
# Note that generated sources which do not exist when this snippet is
# loaded will not be depped. Since that normally only happens on
# a first-time build, when everything is re-built anyway, this is
# not a practical problem.

DEPS_C_SOURCES_GLOB ?= *.cpp *.c *.c++ *.C *.cc *.moc *.h *.hpp *.h++ *.hxx
SOURCE_FILES_TO_DEP ?= $(sort $(wildcard $(DEPS_C_SOURCES_GLOB)))

TOC_C_DEPS_MAKEFILE = $(TOC_MAKESDIR)/C_DEPS.make
TOC_MKDEP_SRC = $(TOC_HOME)/bin/mkdep.c
TOC_MKDEP_BIN = $(top_srcdir)/mkdep
$(TOC_MKDEP_BIN): $(TOC_C_DEPS_MAKEFILE) $(TOC_MKDEP_SRC)
	@$(call toc_compile_c_binary,$@,$(TOC_MKDEP_SRC))
DISTCLEAN_FILES += $(TOC_MKDEP_BIN)

ifneq (,$(SOURCE_FILES_TO_DEP))
# We've got sources. Let's dep 'em...
TOC_C_DEPSFILE = .toc.C_DEPS.make
$(TOC_C_DEPSFILE): $(TOC_MKDEP_BIN) $(SOURCE_FILES_TO_DEP) Makefile $(TOC_C_DEPS_MAKEFILE)
ifneq (,$(strip $(filter clean distclean,$(MAKECMDGOALS))))
	@echo "$(MAKECMDGOALS): skipping C_DEPS rules generation."
else
	@test -n "$(SOURCE_FILES_TO_DEP)" || exit 0; \
	echo "Generating C_DEPS rules for $(DEPS_C_SOURCES_GLOB)"; \
	$(call toc_make_c_deps,$(SOURCE_FILES_TO_DEP),)  > $@
-include $(TOC_C_DEPSFILE)
endif
C_DEPS: $(TOC_C_DEPSFILE)
deps: C_DEPS
CLEAN_FILES += $(TOC_C_DEPSFILE)

all: C_DEPS

else
# no sources to dep. Set up a dummy target.
C_DEPS:

endif

