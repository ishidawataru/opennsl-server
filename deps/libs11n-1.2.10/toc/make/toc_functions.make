#!make
# ^^^ let's help out emacs :/
# this file contains more-or-less generic call()able Make functions.

######################################################################
# toc_atparse_file call()able:
# Similar to the toc shell function of the same name, except for different arguments:
# Filters an input file using toc's at-parser (e.g., @TOKENS@), and sends output to $2.
# $2 is only updated if the generated output is different, so it is dependencies-safe.
# Args:
# $1 = input template
# $2 = output file
# $3 = list of properties, e.g. FOO=bar BAR=foo
TOC_ATPARSER_BIN = $(TOC_BINDIR)/atsign_parse
ifeq (,$(wildcard $(TOC_ATPARSER_BIN)))
$(error Could not find TOC_ATPARSER_BIN at $(TOC_ATPARSER_BIN)!)
endif
toc_atparse_file = \
        echo -n 'toc_atparse_file: '; $(TOC_ATPARSER_BIN) $(3) < $(1) > $(2).tmp; \
                if cmp -s $(2).tmp $(2); then rm -f $(2).tmp; echo "$(2) is up to date."; \
                else mv -f $(2).tmp $(2); echo "Updated $(2)."; fi


######################################################################
# toc_generate_rules() callable:
#   $1 = rules name, e.g., INSTALL_XXX, BIN_PROGRAMS, etc.
#   $2 = arguments to pass to makerules.$(1)
# Sample:
# .toc.foo.deps:
#     $(toc_generate_rules,foo,arg1=val1 -arg2)
# Executes script $(TOC_MAKESDIR)/makerules.foo, passing it $(2).
toc_generate_rules = $(SHELL) $(TOC_MAKESDIR)/makerules.$(1) $(2)
# the $(SHELL) prefix here is a kludge: systems where /bin/sh is Bourne
# fail to build, but the nature of the errors makes it quite difficult
# to track down where the failure is. The side-effect of this kludge
# is that makerules.foo must be a shell script. :/

######################################################################
# toc_get_makefile_var call()able:
# $1 = VAR_NAME
# $2 = optional filename (defaults to Makefile)
toc_get_makefile_var = $(shell $(TOC_HOME)/bin/getMakefileVar $(1) $(2))



######################################################################
# toc_compile_c_binary call()able function:
# compiles/links list of source files [$($(1)_bin_SOURCES) $($(1)_bin_OBJECTS)and $(2)]
# to create $(1). It uses the optional $($(1)_bin_CFLAGS).
toc_compile_c_binary = $(CC) $(C_BINS_FROM_SOURCES_CFLAGS) $(CFLAGS) $($(1)_bin_CFLAGS) -o $(1) $($(1)_bin_SOURCES) $($(1)_bin_OBJECTS) $(2) $(C_BINS_FROM_SOURCES_LFLAGS) $(LDFLAGS) $($(1)_bin_LFLAGS)
# does $(LDFLAGS) /really/ belong here???

######################################################################
# toc_made_c_deps call()able function:
# generates make dependencies for c/c++ files.
# $1 = C/C++ sources to process.
# $2 = INCLUDES (e.g., -I. -I/usr/include, with or without the -I).
#      $2 defaults to $(INCLUDES).
toc_make_c_deps = inc="$(patsubst %,-I%,$(wildcard $(patsubst -I%,%,$(2))))"; \
		test "x$$inc" = "x" && inc="$(INCLUDES)"; \
		$(TOC_MKDEP_BIN) $$inc -- $(1)
# the $(wildcard) oddness above is to remove duplicate entires and invalid
# dirs (mkdeps no likie invalid ones).
# maintenance note: $(TOC_MKDEP_BIN) comes from C_DEPS.make


ifneq (,$(CCDV_BIN))
    COMPILE_COMMAND_QUIET_PREFIX = @
else
    COMPILE_COMMAND_QUIET_PREFIX =
endif

######################################################################
# Experimental override of %.o:%.cpp
COMPILE_COMMAND_CXX = $(CXX) $(CXXFLAGS) $($(subst .,_,$<)_CXXFLAGS) \
	$(CPPFLAGS) $($(subst .,_,$<)_CPPFLAGS) -c -o $@ $<
# ^^^ ..._CPPFLAGS == e.g., main.cpp.CPPFLAGS or main_cpp_CPPFLAGS
%.o: %.cpp
	$(COMPILE_COMMAND_QUIET_PREFIX)$(COMPILE_COMMAND_CXX)
######################################################################
# Experimental override of %.o:%.c
COMPILE_COMMAND_C = $(CC) $(CFLAGS)  $($(subst .,_,$<)_CFLAGS) \
	$(CPPFLAGS) $($(subst .,_,$<)_CPPFLAGS) -c -o $@ $<
%.o: %.c
	$(COMPILE_COMMAND_QUIET_PREFIX)$(COMPILE_COMMAND_C)

