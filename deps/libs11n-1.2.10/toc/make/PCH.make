#!/do/not/make
############################################################
# EXPERIMENTAL gcc PCH support (20.11.2003)
#
# Use in conjunction with the gcc_pch toc test.
#
# Clients must define:
#
# PRECOMPILED_HEADERS_CXX = list of C++ headers
# PRECOMPILED_HEADERS_C = list of C headers
#
#
# Optionally, set:
#   PCH_SUFFIX = filename suffix for PCH files (defaults to .gch).
#
# Running the PCH target will create *$(PCH_SUFFIX) for each *.h
# in PRECOMPILED_HEADERS_{C,CXX}.
#
# After including this file, the variable USE_PCH will be set to 1 if
# the client can expect PCH files to be generated. This can be used, e.g.,
# to conditionally add or replace [pre]installed headers with PCH files.
#

PCH_MAKEFILE = $(TOC_MAKESDIR)/PCH.make

ifneq (1,$(GCC_SUPPORTS_PCH))
PCH:
	@echo "$(PCH_MAKEFILE): GCC_SUPPORTS_PCH is not set to 1, so PCH support is disabled."
USE_PCH = 0
else

PCH_SUFFIX ?= .gch

COMPILE_COMMAND_CXX_PCH = $(CXX) $(CXXFLAGS) \
	$(CPPFLAGS) -xc++ $(TARGET_ARCH) -c -o $(1) $(patsubst %$(PCH_SUFFIX),%,$(1))

COMPILE_COMMAND_C_PCH = $(CC) $(CFLAGS) \
	$(CPPFLAGS) -xc $(TARGET_ARCH) -c -o $(1) $(patsubst %$(PCH_SUFFIX),%,$(1))


PCH_CXX = $(addsuffix $(PCH_SUFFIX),$(PRECOMPILED_HEADERS_CXX))
ifneq (,$(PCH_CXX))
CLEAN_FILES += $(PCH_CXX)
USE_PCH ?= 1
$(PCH_CXX):
	@$(call COMPILE_COMMAND_CXX_PCH,$@)
endif
# ^^^ PCH_CXX


PCH_C = $(patsubst %.h,%$(PCH_SUFFIX),$(PRECOMPILED_HEADERS_C))
ifneq (,$(PCH_C))
CLEAN_FILES += $(PCH_C)
USE_PCH ?= 1
$(PCH_C):
	@$(call COMPILE_COMMAND_C_PCH,$@)
endif
# ^^^ PCH_C
PCH_RULES = .toc.PCH.make
$(PCH_RULES): $(PCH_MAKEFILE) Makefile
	@echo "Creating PCH rules."
	@echo '' > $@
	@for x in $(PCH_C) $(PCH_CXX); do \
		foo=$$x; foo=$${foo%%$(PCH_SUFFIX)}; \
		echo "$$x: $$foo" >> $@; \
	done

PCH: $(PCH_RULES) $(PCH_CXX) $(PCH_C)

endif
# ^^^^^^^ GCC_SUPPORTS_PCH

USE_PCH ?= 0
