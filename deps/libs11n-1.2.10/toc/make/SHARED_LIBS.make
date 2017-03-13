#!/do/not/make
# Links objects into a dynamically-linked library using $(CXX)
# (only tested with g++).
#
# Usage: 
#
# SHARED_LIBS = foo
# SHARED_LIBS_OBJECTS = # optional list of .o files to link to all $(SHARED_LIBS)
# # optional: SHARED_LIBS_INSTALL = 0# If you do NOT want INSTALL rules creates for any
#                                      of the $(SHARED_LIBS). Default = 1.
# foo_so_OBJECTS = list of object files to link to foo.so
# foo_so_LDADD = # optional libraries passed to linker (e.g., -lstdc++)
# foo_so_LDFLAGS = # optional args to linker
# foo_so_VERSION = # optional Major.Minor.Patch # e.g. "1.0.1" it MUST match this format, but
#                    need not be numeric. Default is no version number
# foo_so_INSTALL_DEST = # optional installation path. Defaults to $(INSTALL_LIBEXECS_DEST)
# foo_so_INSTALL = 0 # optional: suppresses generation of installation rules.
#                    i.e., foo.so will not be installed. Default = $(SHARED_LIBS_INSTALL)
# foo_so_SONAME = foo.so.NUMBER # optional: explicitely defines the -soname tag
# include $(TOC_MAKESDIR)/SHARED_LIBS.make
#
# If foo_so_VERSION is not set then no version number is used, and the
# conventional "versioned symlinks" are not created. This is normally
# used when linking plugins, as opposed to full-fledged libraries.
#
# Run:
#    all: SHARED_LIBS
#
# Effect:
# Creates foo.so, and optionally foo.so<version>, by linking
# $(foo_so_OBJECTS). It also sets up install/uninstall rules for
# handling the various version-number symlinks conventionally used
# to link mylib.so.1.2.3 to mylib.so.
########################################################################
SHARED_LIBS_MAKEFILE = $(TOC_MAKESDIR)/SHARED_LIBS.make

########################################################################
# toc_link_shared_lib call()able function
# $1 = basename
# $2 = extra args for the linker
toc_link_shared_lib = { soname=$(1).so; wholename=$${soname}.$($(1)_so_MAJOR).$($(1)_so_MINOR).$($(1)_so_PATCH); \
			if test x = "x$($(1)_so_SONAME)"; then stamp=$${soname}.$($(1)_so_MAJOR); \
			else stamp=$($(1)_so_SONAME); fi; \
			test x = "x$($(1)_so_MAJOR)" && { wholename=$${soname}; stamp=$(1).so;}; \
			cmd="$(CXX) $(CXXFLAGS) $(LDFLAGS) $($(1)_LDFLAGS) -o $${wholename} -export-dynamic -shared \
				-Wl,-soname=$${stamp} $($(1)_so_OBJECTS) $(SHARED_LIBS_OBJECTS) $(SHARED_LIBS_LDADD) $($(1)_so_LDADD) $(2)"; \
			test x = "x$(CCDV_BIN)" && echo $$cmd; \
			$$cmd || exit; \
			test x = "x$($(1)_so_MAJOR)" || { \
				ln -fs $${wholename} $${soname}.$($(1)_so_MAJOR).$($(1)_so_MINOR); \
				ln -fs $${wholename} $${soname}.$($(1)_so_MAJOR); \
				ln -fs $${wholename} $${soname}; \
				}; \
			}
# symlinking methods:
# method 1:
#			ln -fs $${wholename} $${soname}.$($(1)_so_MAJOR).$($(1)_so_MINOR); \
#			ln -fs $${soname}.$($(1)_so_MAJOR).$($(1)_so_MINOR) $${soname}.$($(1)_so_MAJOR); \
#			ln -fs $${soname}.$($(1)_so_MAJOR) $${soname}; \
# method 1.5:
#			link1=$${soname}.$($(1)_so_MAJOR).$($(1)_so_MINOR); ln -fs $${wholename} $$link1; \
#			link2=$${soname}.$($(1)_so_MAJOR); ln -fs $$link1 $$link2; \
#			link3=$${soname}; ln -fs $$link2 $$link3; \
# method 2:
#			ln -fs $${wholename} $${soname}.$($(1)_so_MAJOR).$($(1)_so_MINOR); \
#			ln -fs $${wholename} $${soname}.$($(1)_so_MAJOR); \
#			ln -fs $${wholename} $${soname}; \


#			for l in $${soname} \
#				$${soname}.$($(1)_so_MAJOR) \
#				$${soname}.$($(1)_so_MAJOR).$($(1)_so_MINOR) \
#				; do \
#				ln -fs $${wholename} $$l; \
#			done; \

ifneq ($(SHARED_LIBS),)
SHARED_LIBS_SOFILES = $(patsubst %,%.so,$(SHARED_LIBS))

SHARED_LIBS_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.SHARED_LIBS

SHARED_LIBS_DEPSFILE = .toc.SHARED_LIBS.make
$(SHARED_LIBS_DEPSFILE): Makefile $(SHARED_LIBS_MAKEFILE) $(SHARED_LIBS_RULES_GENERATOR)
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping SHARED_LIBS rules generation."
else
	@echo "Generating SHARED_LIBS rules."; \
	$(call toc_generate_rules,SHARED_LIBS,$(SHARED_LIBS)) > $@
endif
-include $(SHARED_LIBS_DEPSFILE)

CLEAN_FILES += $(SHARED_LIBS_SOFILES) $(wildcard $(patsubst %,%.*,$(SHARED_LIBS_SOFILES)))

libs: SHARED_LIBS

endif
# ^^^ got SHARED_LIBS?
