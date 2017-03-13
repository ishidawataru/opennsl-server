#!/do/not/make

# a (big) snippet to build API docs using doxygen
# Much of this process was written by Rusty Ballinger.
#

# Edit the file Doxyfile template, Doxyfile.at, using
# @at-token@ conventions.
#
# Define:
#  DOXYGEN_PREDEF = predfined C vars
#  DOXYGEN_ATPARSE_ARGS = list of args to pass to the @at-parser@
#      Some globally-defined vars are pre-set.
#      In particular you may want to add:
#       DOXYGEN_INPUT="index.txt $(top_includesdir)/yourincludes)"
#  DOXYGEN_FILTER=/path/to/filter (may be left blank)
#      Useful sometimes for perl/sed'ing, e.g., C macros in API docs.
#
# Sample:
#  INCLUDES_DIRS = $(addprefix $(top_includesdir)/,lib1 otherlib)
#
#  DOXYGEN_PREDEF = \
#	HAVE_CONFIG_H=1
#
#  DOXYGEN_ATPARSE_ARGS = \
#	DOXYGEN_INPUT="index.txt $(INCLUDES_DIRS)"
#
# Of course your Doxyfile.at must have the appropriate @tokens@ in it,
# but the one shipped with this file is already set up for you.

ifeq (,$(DOXYGEN_BIN))
$(error The variable DOXYGEN_BIN must be set before including this file.)
endif

DOXYFILE = Doxyfile

DOXYGEN_HTML_OUTDIR  ?= ./doxygen
DOXYGEN_IN = Doxyfile.at
DIST_FILES += $(DOXYGEN_IN) index.txt

DOXYGEN_INSTALL_DIRNAME ?= doxygen-$(PACKAGE_NAME)-$(PACKAGE_VERSION)
docs: doxygen
clean: clean-doxygen
install: install-doxygen


DOXYGEN_ATPARSE_ARGS += \
	top_srcdir="${top_srcdir}" \
	top_libdir="${top_libdir}" \
	top_includesdir="${top_includesdir}" \
	PERL="$(PERL)" \
	PACKAGE_NAME="$(PACKAGE_NAME)" \
	PACKAGE_VERSION="$(PACKAGE_VERSION)" \
	PREDEFINED="$(DOXYGEN_PREDEF)" \
	DOXYGEN_FILTER="$(DOXYGEN_FILTER)" \
	DOXYGEN_HTML_OUTDIR="$(DOXYGEN_HTML_OUTDIR)"


DOXYGEN_MAKE = $(toc_makesdir)/DOXYGEN.make

atparser = $(top_srcdir)/toc/bin/atsign_parse
$(top_srcdir)/configure:
$(DOXYFILE): $(DOXYGEN_FILTER) $(DOXYGEN_IN) Makefile $(top_srcdir)/configure
	@echo "Creating $@."; \
	$(call toc_atparse_file,$(DOXYGEN_IN),$@, \
		$(DOXYGEN_ATPARSE_ARGS) \
	)

CLEAN_FILES += $(DOXYFILE)

INSTALL_DEST_DOXYGEN = $(INSTALL_DOCS_DEST)

DISTCLEAN_FILES += $(DOXYFILE)
doxygen: clean-doxygen $(DOXYFILE)
	$(DOXYGEN_BIN) 
	@echo HTML output is in $(DOXYGEN_HTML_OUTDIR).
clean-doxygen:
	-rm -fr $(DOXYGEN_HTML_OUTDIR) latex
doxygen_finaldest = $(INSTALL_DOCS_DEST)/$(DOXYGEN_INSTALL_DIRNAME)
install-doxygen: doxygen
	@echo "Installing HTML docs to $(doxygen_finaldest)"
	@test -d $(doxygen_finaldest) && exit 0; rm -fr $(doxygen_finaldest)
	@test -d $(INSTALL_DOCS_DEST) && exit 0; mkdir -p $(INSTALL_DOCS_DEST)
	@cp -r $(DOXYGEN_HTML_OUTDIR) $(doxygen_finaldest)

uninstall-doxygen:
	@echo "Uninstalling doxgen-generated docs: $(doxygen_finaldest)"
	@-test -d $(doxygen_finaldest) || exit 0; rm -fr $(doxygen_finaldest)

uninstall: uninstall-doxygen

# all: docs
