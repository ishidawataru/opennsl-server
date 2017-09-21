#!/usr/bin/make -f
# Usage:
#
#   CLEAN_FILES += *~ foo bar
#   DISTCLEAN_FILES += nekkid_pic_of_mrs_walters.jpg
#   include path/to/this/file
#
# That creates 'clean' and 'distclean' targets which will nuke those files/dirs.
#
# Special case: cleaning subdirs in an order other than that specified in $(SUBDIRS):
#
# Set TOC_OVERRIDE_CLEAN to 1 /before including toc.make/ in any given
# makefile in order to have 'clean' clean/distclean targets, i.e., one you can
# re-order all you want by using dependencies.
# e.g.:
#    clean: $(addprefix clean-,bin sbin lib doc tests .)
# or:
#    clean: precleaner subdirs-clean clean-. aftercleaner
#
# If you do this the default clean/distclean behaviour will be to do NOTHING,
# so you must be sure to set the dist/clean deps.


.PHONY: clean distclean clean-.

CLEAN_FILES += $(DEPS_FILE) $(DIST_LISTFILE)

######################################################################
# toc_clean_files call()able function: deletes all files passed in via
# $(1).
toc_clean_files = test "x$(1)" = "x"  && exit 0; \
		for x in $(1) ""; do test -z "$${x}" && continue; \
			test -w $$x || continue; \
			rmargs=""; test -d $$x && rmargs="-rf"; \
			rm $$rmargs $$x || exit; \
		done; \
		exit 0


clean-%: # assume $* is a subdirectory
	@$(call toc_make_subdirs,$*,clean)
distclean-%: # assume $* is a subdirectory
	@$(call toc_make_subdirs,$*,distclean)


clean-.: clean-subdirs
ifeq (1,$(configure_build_quietly))
	@echo "Cleaning up..."; \
	$(call toc_clean_files,$(wildcard $(CLEAN_FILES)))
else
	@echo "Cleaning up: $(wildcard $(CLEAN_FILES))"; \
	$(call toc_clean_files,$(wildcard $(CLEAN_FILES)))
# i hate the duplicate wildcard, but i can't pass shell-expanded vars
# to $(call ...).
endif

distclean-.: distclean-subdirs
	@echo -n "Cleaning up... "
ifeq (1,$(configure_build_quietly))
	@$(call toc_clean_files,$(wildcard $(CLEAN_FILES) $(DISTCLEAN_FILES))); \
		echo
else
	@echo $(wildcard $(CLEAN_FILES) $(DISTCLEAN_FILES)); \
	$(call toc_clean_files,$(wildcard $(CLEAN_FILES) $(DISTCLEAN_FILES)))
endif

subdirs-clean: # implemented elsewhere
subdirs-distclean: # implemented elsewhere
clean-subdirs: subdirs-clean
distclean-subdirs: subdirs-distclean


# toc-XXX stubs: don't use these directly
#toc-clean: subdirs-clean clean-.
#toc-distclean: subdirs-distclean distclean-.
# we clean depth-first to keep some top-level generated files from being nuked :/

ifeq (1,$(TOC_OVERRIDE_CLEAN))
clean:
distclean:
else
clean: clean-.
distclean: distclean-.
endif
