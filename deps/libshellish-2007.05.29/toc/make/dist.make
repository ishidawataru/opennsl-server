# Snippet intended to be included from a toc-assisted Makefile.
# It powers a 'make dist' functionality which uses this var:
#  DIST_FILES
# it takes a list of files/dirs to create a distribution tarball from.
# This snippet requires a filesystem-valid $(PACKAGE_NAME)-$(PACKAGE_VERSION)
# combination
#
# You may set several configure_dist_xxx vars to configure which packages it
# creates - see the code below for details.

DIST_FILES_FILE = $(top_srcdir)/.toc.DIST_FILES
CLEAN_FILES += $(wildcard $(DIST_FILES_FILE))


# dist_find_bin: helper function to find $(1) in an adjusted PATH
dist_find_bin = $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(toc_tocdir)/bin:$(PATH)))))

TOC_MAKEDIST_BIN ?= $(call dist_find_bin,makedist)
ifeq (,$(wildcard $(TOC_MAKEDIST_BIN)))
$(error TOC_MAKEDIST_BIN must be set to point to the makedist application.)
endif


ifeq (.,$(top_srcdir))
dist: clean_DIST_FILES_FILE
ifneq (,$(wildcard $(DIST_FILES_FILE)))
clean_DIST_FILES_FILE: FORCE
	rm $(DIST_FILES_FILE)
else
clean_DIST_FILES_FILE:
endif
# ^^^ clean up old DIST_FILES_FILE
endif
# ^^^^ in $(top_srcdir)


dist-.: $(DIST_FILES_FILE)

$(DIST_FILES_FILE): FORCE Makefile
	@test "x" = "x$(DIST_FILES)" && exit 0; \
		pwd=$$(pwd); \
		echo "Adding DIST_FILES to [$(DIST_FILES_FILE)]..."; \
		for f in "" $(DIST_FILES); do test "x" = "x$$f" && continue; \
			echo $$pwd/$$f; \
		done >> $@;

# dist: $(DIST_FILES_FILE)
dist: dist-subdirs dist-.

ifeq (.,$(top_srcdir))

DIST_NAME = $(PACKAGE_NAME)-$(PACKAGE_VERSION)
DIST_FILES_TARBALL = $(DIST_NAME).tar
CLEAN_FILES += $(wildcard $(DIST_FILES_TARBALL))
dist-tar: $(DIST_FILES_TARBALL)
$(DIST_FILES_TARBALL): $(DIST_FILES_FILE)
	@echo "Making dist tarball..."; \
		cd $(top_srcdir) > /dev/null; \
		top=$$(pwd); \
		cd - > /dev/null; \
		tmpfile=.toc.DIST_FILES.toplevel; \
		cat $(DIST_FILES_FILE)  >> $$tmpfile; \
			perl -pe "s|$${top}/||" $$tmpfile > foo.bar; \
				sort < foo.bar > $$tmpfile; rm foo.bar; \
		$(TOC_MAKEDIST_BIN) $$tmpfile $@ || exit; \
		rm $$tmpfile $(DIST_FILES_FILE)

dist-compress:
dist-.: $(DIST_FILES_TARBALL) dist-compress

GZIP_BIN ?= $(call dist_find_bin,gzip)
ifneq (,$(GZIP_BIN))
configure_dist_use_gzip ?= 1
endif

BZIP_BIN ?= $(call dist_find_bin,bzip2)
ifneq (,$(BZIP_BIN))
configure_dist_use_bzip ?= 1
endif

ZIP_BIN ?= $(call dist_find_bin,zip)
ifneq (,$(ZIP_BIN))
configure_dist_use_zip ?= 1
endif

ifeq (1,$(configure_dist_use_gzip))
CLEAN_FILES += $(wildcard $(DIST_FILES_TARBALL).gz)
dist-compress: dist-tarball-gzip
dist-gzip: dist dist-tarball-gzip
dist-tarball-gzip: $(DIST_FILES_TARBALL).gz
$(DIST_FILES_TARBALL).gz: $(DIST_FILES_TARBALL)
	$(GZIP_BIN) -c $< > $@
endif
# ^^^ end gzip

ifeq (1,$(configure_dist_use_bzip))
CLEAN_FILES += $(wildcard $(DIST_FILES_TARBALL).bz2)
dist-compress: dist-tarball-bzip
dist-bzip: dist dist-tarball-bzip
dist-tarball-bzip: $(DIST_FILES_TARBALL).bz2
$(DIST_FILES_TARBALL).bz2: $(DIST_FILES_TARBALL)
	$(BZIP_BIN) -c $< > $@
endif
# ^^^ end bzip

ifeq (1,$(configure_dist_use_zip))
DIST_TARBALL_ZIP = $(DIST_NAME).zip
CLEAN_FILES += $(wildcard $(DIST_TARBALL_ZIP))
dist-compress: dist-tarball-zip
dist-zip: dist dist-tarball-zip
dist-tarball-zip: $(DIST_TARBALL_ZIP)
$(DIST_TARBALL_ZIP): $(DIST_FILES_TARBALL)
	@test -d $(DIST_NAME) && rm -fr $(DIST_NAME); true
	@tar xf $<
	$(ZIP_BIN) -qr $@ $(DIST_NAME)
	@rm -fr $(DIST_NAME)
endif
# ^^^ end infozip


dist-compress: dist-cleanup dist-showzips
dist-showzips:
	@ls -la $(DIST_NAME).*
	@echo "archived file count: $$(tar tf $(DIST_FILES_TARBALL) | wc -l)"

dist-cleanup:
	@echo "Cleaning up after dist..."
	@rm -fr $(DIST_NAME)
	@files="$(find . -name $(DIST_FILES_FILE))"; test -z "$$files" && exit 0; \
		echo $$files | xargs rm

testdist: dist
	tar xf $(DIST_FILES_TARBALL)
	cd $(DIST_NAME); \
		./configure $(DIST_TESTDIST_ARGS) && make || exit $$?
	@echo "$@ $(PACKAGE_NAME)-$(PACKAGE_VERSION) completed. $(TOC_EMOTICON_OKAY)"

MD5_BIN ?= $(call dist_find_bin,md5sum*)
ifeq (,$(MD5_BIN))
$(warning MD5_BIN not pointing to md5sum application. Skipping md5-related checks.)
else
MD5_LIST = md5.sums.$(DIST_NAME)
md5check-%:
	@echo "Checking distribution md5 sums against $*..."
	@$(MD5_BIN) --check $* | grep FAILED || exit 0 && exit 1
	@echo "${TOC_EMOTICON_OKAY}"
md5check: md5check-$(MD5_LIST)

endif
# ^^^ end md5 stuff

endif
# ^^^ end of $(top_srcdir) block

dist-subdirs:
	@$(call toc_make_subdirs,$(SUBDIRS),dist)

