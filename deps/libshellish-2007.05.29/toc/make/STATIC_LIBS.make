# cpp_static_libs.make: snippet for linking static libraries from
# object files.
#
# This snippet defines the targets 'libs-static' and 'X.a' for each X in
# $(STATIC_LIBS). libs-static simply depends on X.a.
#
# sample usage:
#
#     STATIC_LIBS = foo bar
#     STATIC_LIBS_OBJECTS = # optional list of .o files to link to all $(STATIC_LIBS)
#     foo_a_OBJECTS = list of .o files
#     bar_a_OBJECTS = list of .o files
#
#     all: libs
#(or) all: foo.a bar.a
#
# that creates foo.a and bar.a and adds those files to $(INSTALL_LIBS)

STATIC_LIBS_MAKEFILE = $(TOC_MAKESDIR)/STATIC_LIBS.make
STATIC_LIBS_A = $(patsubst %,%.a,$(STATIC_LIBS))
CLEAN_FILES += $(STATIC_LIBS_A)

# toc_link_static_lib call()able:
# $1 = base name (e.g., foo)
# links $(1).a from $($(1)_a_OBJECTS) and $(STATIC_LIBS_OBJECTS)
toc_link_static_lib = $(AR) crs $(1).a $($(1)_a_OBJECTS) $(STATIC_LIBS_OBJECTS)

ifneq (,$(STATIC_LIBS_A))

STATIC_LIBS_DEPSFILE = .toc.STATIC_LIBS.make
STATIC_LIBS_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.STATIC_LIBS

STATIC_LIBS:

$(STATIC_LIBS_DEPSFILE): Makefile $(STATIC_LIBS_MAKEFILE) $(STATIC_LIBS_RULES_GENERATOR)
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping STATIC_LIBS rules generation."
else
	@echo "Generating STATIC_LIBS rules."; \
	$(call toc_generate_rules,STATIC_LIBS,$(STATIC_LIBS)) > $@
endif
-include $(STATIC_LIBS_DEPSFILE)
deps: $(STATIC_LIBS_DEPSFILE)

libs: STATIC_LIBS
libs-static: STATIC_LIBS

endif
