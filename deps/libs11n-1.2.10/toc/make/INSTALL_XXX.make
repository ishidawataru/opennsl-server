#!/do/not/make
#
# To be included from the shared toc makefile.
# Creates un/install rules for sets of files and defines some
# toc-wide $(call)able functions for un/installing files.
#
# Sample usage:
#  INSTALL_BINS = mybin myotherbin # installs to $(prefix)/bin
#  INSTALL_LIBS = mylib.a myotherlib.a # installs to $(prefix)/lib
#
# There's a *whole lot* more to know, if you wanna poke around the code.
#
# Design note: the traditional xxx-local targets aren't really
# necessary. If someone wants to customize install they can simply do
# this:
#
# install: my-install
# my-install:
#      ....
#
# If you need a pre-install hook, either implement this target:
#
# install-.:  # (that's "." as in $PWD)
#
# or simply use dependency orderings to define one:
#
# myrules:
# install: myrules
#
#
# For each X in (some list you can find in this makefile) it
# implements the following targets:
#
#  install-X:
#  uninstall-X:
#  install-X-symlink:
#
# Files will be installed to $(INSTALL_X_DEST) using install
# arguments $(INSTALL_X_INSTALL_FLAGS). All of these vars are
# set up by default, but may be customized:
#
#  INSTALL_X_DEST = $(prefix)/$(PACKAGE_NAME)/bin
#  INSTALL_X_INSTALL_FLAGS = -m 0775
#
# To add new installation groups to this file do the following:
#
# - Add an MYNEWGROUP='install/path' entry to the var INSTALL_XXX_PATHS.
#
# - Add INSTALL_MYNEWGROUP_INSTALL_FLAGS var, following the conventions
#   set by the other ...INSTALL_FLAGS vars in this file.
#
# Installation can be further customized by using the toc_make_xxx
# call()able functions defined below...
#
########################################################################

ifeq (,$(wildcard $(INSTALLER_BIN)))
$(error INSTALL_XXX.make requires that the variable INSTALLER_BIN point to a version of install which is vaguely compatible with GNU install. \
	Normally it will be set by the toc_core_tests configure test.)
endif

#INSTALL_BINS_SUFFIX ?=
#ifeq (1,$(configure_with_CYGWIN))
#INSTALL_BINS_SUFFIX = .exe
#endif

############################################################
# toc_make_install call()able:
# $1=file list
# $2=destdir. The global $(DESTDIR) is prepended to it
# $3=flags for $(INSTALLER_BIN)
toc_make_install = test "x$(1)" = x && exit 0; \
			dest="$(DESTDIR)$(2)"; \
			test -d "$$dest" || mkdir -p "$$dest" || exit; \
			for b in $(1) "x"; do test "x$$b" = "xx" && break; \
				b=$$(basename $$b); \
				target="$$dest/$$b"; \
				cmd="$(INSTALLER_BIN) $(3) $$b $$target"; echo $$cmd; $$cmd || exit; \
			done

############################################################
# toc_make_install_update: identical to toc_make_install
# but does not update the target if it is the same as the source.
# It uses 'cmp' to compare the files, not timestamps, and
# is a lot slower than doing a normal/overwriting install.
toc_make_install_update = test x = "x$(1)" && exit 0; \
			dest="$(DESTDIR)$(2)"; \
			test -d "$$dest" || mkdir -p "$$dest" || exit; \
			for b in $(1) ""; do test -z "$$b" && continue; \
				b="$$(basename $$b)"; \
				target="$$dest/$$b"; \
				cmp "$$target" "$$b" > /dev/null 2>&1  && continue; \
				cmd="$(INSTALLER_BIN) $(3) $$b $$target"; echo "$$cmd"; $$cmd || exit; \
			done

############################################################
# toc_make_uninstall call()able:
# removes all files listed in $(1) from target directory $(2).
# Files in $(1) which do not exist are silently ignored.
# Note that it does not use 'rm -fr', so $(1) entries must
# be read/writable files, not directories.
toc_make_uninstall =  test "x$(1)" = x && exit 0; \
			dest="$(DESTDIR)$(2)"; \
			test -e "$$dest" || exit 0; \
			for b in $(1) ""; do test -z "$$b" && continue; \
				fp="$$dest/$$b"; test -e "$$fp" || continue; \
				cmd="rm $$fp"; echo $$cmd; $$cmd || exit $$?; \
			done

############################################################
# toc_make_install_symlink call()able:
# Works similarly to toc_make_install, but symlinks back to the
# install source, instead of copying. Arg $3 is ignored.  Note that
# symlinks must be linked to absolute paths here, because we cannot
# easily/reliably make a relative path from the target directory back
# to the install source:
toc_make_install_symlink = test "x$(1)" = x && exit 0; \
			dest="$(DESTDIR)$(2)"; \
			test -d "$$dest" || mkdir -p "$$dest" || exit; \
			for b in $(1) ""; do test -z "$$b" && continue; \
				target="$$dest/$$b"; \
				pwd="$$(pwd)"; \
				src="$$pwd/$$b"; \
				test "$$target" -ef "$$src" && continue; \
				test -f "$$target" && rm "$$target"; \
				echo "Symlinking $$target"; ln -s -f "$$src" "$$target" || exit $$?; \
			done

############################################################
# toc_make_install_so: installs foo.so.X.Y.Z and symlinks foo.so, foo.so.X and foo.so.X.Y to it,
# in traditional/common Unix style.
# $1 = so name (foo.so)
# $2-4 = Major, Minor, Patch version numbers
# $5 = destination directory
toc_make_install_so =  test "x$(1)" = x && exit 0; \
			dest="$(DESTDIR)$(5)"; \
                        test -d "$$dest" || mkdir -p "$$dest" || exit; \
                        wholename=$(1).$(2).$(3).$(4); \
                        target="$$dest"/$$wholename; \
			test $$wholename -ef $$target || { \
	                        echo "Installing/symlinking $$target"; \
				cmd="$(INSTALLER_BIN) -m 0755 $$wholename $$target"; \
				$$cmd || exit; \
			}; \
			cd "$$dest"; \
			for i in $(1) $(1).$(2) $(1).$(2).$(3); do \
				test -e $$i && rm $$i; \
				cmd="ln -fs $$wholename $$i"; echo $$cmd; \
				$$cmd || exit; \
			done
# symlinking method number 2:
#			{ set -x; \
#				ln -fs $(1).$(2).$(3).$(4) $(1).$(2).$(3); \
#				ln -fs $(1).$(2).$(3) $(1).$(2); \
#				ln -fs $(1).$(2) $(1); \
#			}

############# some phony targets...
.PHONY: install-. uninstall-.
# Note, that's "." as in "$PWD"
subdirs-install: # implemented elsewhere
install: install-. install-subdirs
install-update: install-.-update install-subdirs-update
install-symlink: install-.-symlink install-subdirs-symlink
install-subdirs: subdirs-install
install-subdirs-symlink: subdirs-install-symlink
install-subdirs-update: subdirs-install-update
uninstall-subdirs: subdirs-uninstall
uninstall: uninstall-. uninstall-subdirs
# implement these to hook in to the start of the install.
install-.:
install-.-update:
install-.-symlink:
uninstall-.:
#############


########################################################################
# TOC_INSTALL_TARGET_BASENAMES is the list of base names for
# INSTALL_XXX_% vars.  For each entry in that list we must define
# INSTALL_XXX_INSTALL_FLAGS and add an entry to INSTALL_XXX_PATHS,
# both shown below.
TOC_INSTALL_TARGET_BASENAMES += BINS \
				DOCS \
				HEADERS \
				LIBS \
				LIBEXECS \
				MAN1 \
				MAN2 \
				MAN3 \
				MAN4 \
				MAN5 \
				MAN6 \
				MAN7 \
				MAN8 \
				PACKAGE_DATA \
				PACKAGE_HEADERS \
				PACKAGE_LIBS \
				PKGCONFIG \
				SBINS


############ internal shortcuts:
INSTALLER_BIN_FLAGS_BINS = -m 0755
INSTALLER_BIN_FLAGS_NONBINS = -m 0644
INSTALLER_BIN_FLAGS_LIBS = -m 0644
INSTALLER_BIN_FLAGS_LIBEXECS = -m 0755
###########

################# default install flags for the installable file categories:
INSTALL_BINS_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_BINS)
INSTALL_SBINS_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_BINS)
INSTALL_LIBS_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_LIBS)
INSTALL_PACKAGE_LIBS_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_LIBS)
INSTALL_LIBEXECS_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_LIBEXECS)
INSTALL_HEADERS_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_PACKAGE_HEADERS_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_PACKAGE_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_DOCS_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_MAN1_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_MAN2_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_MAN3_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_MAN4_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_MAN5_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_MAN6_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_MAN7_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_MAN8_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
INSTALL_PKGCONFIG_INSTALL_FLAGS ?= $(INSTALLER_BIN_FLAGS_NONBINS)
#################

########################################################################
# installation paths, in the format expected by makerules.INSTALL_XXX
# They should be relative to ${prefix}.
INSTALL_XXX_PATHS += \
		BINS=bin \
		SBINS=sbin \
		LIBS=lib \
		PACKAGE_LIBS='lib/$(PACKAGE_NAME)' \
		LIBEXECS=lib \
		HEADERS=include \
		PACKAGE_HEADERS='include/$(PACKAGE_NAME)' \
		PACKAGE_DATA='share/$(PACKAGE_NAME)' \
		DOCS='share/doc/$(PACKAGE_NAME)' \
		MAN1='share/man/man1' \
		MAN2='share/man/man2' \
		MAN3='share/man/man3' \
		MAN4='share/man/man4' \
		MAN5='share/man/man5' \
		MAN6='share/man/man6' \
		MAN7='share/man/man7' \
		MAN8='share/man/man8' \
		PKGCONFIG='lib/pkgconfig'
# INSTALL_XXX_PATHS Notes: 
#
# - LIBEXECS=lib is intentional: i figure that since there is no /usr/libexec
# nor /libexec, the traditional usage of LIBEXEC is probably "broken".
#
# - PKGCONFIG is data files for the pkg-config tool.
#
# - The difference between HEADERS/LIBS and PACKAGE_HEADERS/LIBS
# is that those with PACKAGE prefix get installed to a directory
# specific to this package, e.g., /usr/include/PACKAGE_NAME/my.h
########################################################################


################ Internal use:
# With make 3.80 we can do this with $(eval). Make 3.79 is still very
# common, but doesn't support $(eval), so we use a shell script to
# generate the rules.
INSTALL_MAKEFILE = $(TOC_MAKESDIR)/INSTALL_XXX.make
INSTALL_RULES_FILE = $(top_srcdir)/.toc.INSTALL_XXX.make
INSTALL_XXX_GENERATOR = $(TOC_MAKESDIR)/makerules.INSTALL_XXX
$(INSTALL_RULES_FILE): $(INSTALL_MAKEFILE) $(INSTALL_XXX_GENERATOR)
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping INSTALL_XXX rules generation."
else
	@echo "Generating rules for INSTALL_XXX."; \
	$(call toc_generate_rules,INSTALL_XXX,\
		$(INSTALL_XXX_PATHS) \
	) > $@
endif
-include $(INSTALL_RULES_FILE)
deps: $(INSTALL_RULES_FILE)
#################
