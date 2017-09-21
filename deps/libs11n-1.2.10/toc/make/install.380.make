# these rules only work with GNU Make 3.80 and higher...
#
#!/bin/make -f
# To be included from the shared toc makefile. REQUIRES GNU install.
#
# Sample usage:
#  INSTALL_BINS = mybin myotherbin # installs to $(prefix)/bin
#  INSTALL_LIBS = mylib.a myotherlib.a # installs to $(prefix)/lib
#
# There's a whole lot more to know, if you wanna poke around the code.
#
# Design note: the traditional xxx-local targets aren't really
# necessary. If someone wants to customize install they can simply do
# this:
#
# install: my-install
# my-install:
#      ....
# 
#
# For each X in $(TOC_INSTALL_TARGET_BASENAMES) the following targets
# are created:
#
#  install-X:
#  uninstall-X:
#  install-X-symlink:
#
# Files will be installed to $(INSTALL_X_DEST) using install
# arguments $(INSTALL_X_INSTALL_FLAGS). All of these vars are
# set up by default, but may be customized:
#
#  INSTALL_BINS_DEST = $(prefix)/$(PACKAGE_NAME)/bin
#  INSTALL_BINS_INSTALL_FLAGS = --mode=0775
#
#
# Installation can be further customized by using the toc_make_xxx
# call()able functions.

TOC_INSTALL_TARGET_BASENAMES = BINS SBINS LIBS PACKAGE_LIBS LIBEXECS HEADERS PACKAGE_HEADERS PACKAGE_DATA DOCS


ifeq (,$(wildcard $(GNUINSTALL_BIN)))
$(error install.make requires that the variable GNUINSTALL_BIN point to GNU install.)
endif


# This whole echo/grep thing is to force it to work on some of my
# older machines where more sane approaches don't seem to work.
TOC_MAKE_INSTALL_GREP_KLUDGE = test $(shell echo $(1) "" | grep -q '[a-zA-Z0-9]'; echo $$?) = 0 || exit 0

# toc_make_install call()able:
# $1=file list, $2=destdir, $3=flags for $(GNUINSTALL_BIN)
toc_make_install = $(call TOC_MAKE_INSTALL_GREP_KLUDGE,$(1)); \
			test -d $(2) || mkdir -p $(2) || exit; \
			for b in $(1) ""; do test -z "$$b" && continue; \
				b=$$(basename $$b); \
				target=$(2)/$$b; \
				cmp $$target $$b > /dev/null 2>&1  && continue; \
				cmd="$(GNUINSTALL_BIN) $(3) $$b $$target"; echo $$cmd; $$cmd || exit; \
			done

# toc_make_uninstall call()able:
# removes all files listed in $(1) from target directory $(2)
toc_make_uninstall =  $(call TOC_MAKE_INSTALL_GREP_KLUDGE,$(1)); \
			test -e "$(2)" || exit 0; \
			for b in $(1) ""; do test -z "$$b" && continue; \
				fp="$(2)/$$b"; test -e "$$fp" || continue; \
				cmd="rm $$fp"; echo $$cmd; $$cmd || exit $$?; \
			done

# toc_make_install_symlink call()able:
# works similarly to toc_make_install, but symlinks back to the install source,
# instead of copying. Arg $3 is ignored.
# Note that symlinks must be linked to absolute paths here, because we cannot
# easily/reliably make a relative path from the target directory back to 
# the install source:
toc_make_install_symlink = $(call TOC_MAKE_INSTALL_GREP_KLUDGE,$(1)); \
			test -d $(2) || mkdir -p $(2) || exit; \
			for b in $(1) ""; do test -z "$$b" && continue; \
				target=$(2)/$$b; \
				test $$target -ef $$b && continue; \
				echo "Symlinking $$target"; ln -s -f $$PWD/$$b $$target || exit $$?; \
			done

# toc_make_install_so: installs foo.so.X.Y.Z and symlinks foo.so, foo.so.X and foo.so.Y to it,
# in traditional/common Unix style.
# $1 = so name (foo.so)
# $2-4 = Major, Minor, Patch version numbers
# $5 = destination directory
toc_make_install_so =  $(call TOC_MAKE_INSTALL_GREP_KLUDGE,$(1)); \
                        test -d $(5) || mkdir -p $(5) || exit; \
                        wholename=$(1).$(2).$(3).$(4); \
                        target=$(5)/$$wholename; \
			test $$wholename -ef $$target || { \
	                        echo "Installing/symlinking $$target"; \
				cmd="$(GNUINSTALL_BIN) -s $$wholename $$target"; \
				$$cmd || exit; \
			}; \
			cd $(5); \
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


GNUINSTALL_FLAGS_NONBINS = --mode=0644
GNUINSTALL_FLAGS_BINS = -s

define INSTALL_MAKE_VARS_TEMPLATE
INSTALL_$(1)_DEST ?= $$(prefix)/$(2)
INSTALL_$(1)_INSTALL_FLAGS ?= $(3)
endef
# set up the initial install locations and install flags:
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,BINS,bin,$(GNUINSTALL_FLAGS_BINS)))
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,SBINS,sbin,$(GNUINSTALL_FLAGS_BINS)))
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,LIBS,lib,$(GNUINSTALL_FLAGS_NONBINS)))
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,PACKAGE_LIBS,lib/$(PACKAGE_NAME),$(GNUINSTALL_FLAGS_NONBINS)))
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,LIBEXECS,libexec,$(GNUINSTALL_FLAGS_NONBINS)))
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,HEADERS,include,$(GNUINSTALL_FLAGS_NONBINS)))
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,PACKAGE_HEADERS,include/$(PACKAGE_NAME),$(GNUINSTALL_FLAGS_NONBINS)))
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,PACKAGE_DATA,share/$(PACKAGE_NAME),$(GNUINSTALL_FLAGS_NONBINS)))
$(eval $(call INSTALL_MAKE_VARS_TEMPLATE,DOCS,share/doc/$(PACKAGE_NAME),$(GNUINSTALL_FLAGS_NONBINS)))


define INSTALL_MAKE_INSTALL_TARGETS
install-$(1):;\
	@$$(call toc_make_install,$$(INSTALL_$(1)),$$(INSTALL_$(1)_DEST),$$(INSTALL_$(1)_INSTALL_FLAGS))
endef
$(foreach foo,$(TOC_INSTALL_TARGET_BASENAMES),$(eval $(call INSTALL_MAKE_INSTALL_TARGETS,$(foo))))

define INSTALL_MAKE_UNINSTALL_TARGETS
uninstall-$(1):;\
	@$$(call toc_make_uninstall,$$(INSTALL_$(1)),$$(INSTALL_$(1)_DEST))
endef
$(foreach foo,$(TOC_INSTALL_TARGET_BASENAMES),$(eval $(call INSTALL_MAKE_UNINSTALL_TARGETS,$(foo))))

define INSTALL_MAKE_INSTALL_SYMLINK_TARGETS
install-$(1)-symlink:;\
	@$$(call toc_make_install_symlink,$$(INSTALL_$(1)),$$(INSTALL_$(1)_DEST))
endef
$(foreach foo,$(TOC_INSTALL_TARGET_BASENAMES),$(eval $(call INSTALL_MAKE_INSTALL_SYMLINK_TARGETS,$(foo))))

install_targets = $(TOC_INSTALL_TARGET_BASENAMES) subdirs
install: $(patsubst %,install-%,$(install_targets))
uninstall: $(patsubst %,uninstall-%,$(install_targets))
install-symlink: $(patsubst %,install-%-symlink,$(install_targets))


install-bins: install-BINS
uninstall-bins: uninstall-BINS

install-sbins: install-SBINS
uninstall-sbins: uninstall-SBINS

install-libs: install-LIBS
uninstall-libs: uninstall-LIBS

install-libexecs: install-LIBEXECS
uninstall-libexecs: uninstall-LIBEXECS

install-pkgdata: install-PACKAGE_DATA
uninstall-pkgdata: uninstall-PACKAGE_DATA

install-headers: install-HEADERS
uninstall-headers: uninstall-HEADERS

install-pkgheaders: install-PACKAGE_HEADERS
uninstall-pkgheaders: install-PACKAGE_HEADERS

install-docs: install-DOCS
uninstall-docs: uninstall-DOCS

install-subdirs:
	@$(call toc_make_subdirs,$(SUBDIRS),install)
install-subdirs-symlink:
	@$(call toc_make_subdirs,$(SUBDIRS),install-symlink)
uninstall-subdirs:
	@$(call toc_make_subdirs,$(SUBDIRS),uninstall)
