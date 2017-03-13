# usage:
# Define a test-local target and do any dir-specific
# tests there. $(SUBDIRS) will be traversed afterwards.

test-subdir-%: FORCE
	@$(call toc_make_subdirs,$*,test)
#	@foo=$@; foo=$${foo##test-}; make -C $$foo test
test-subdirs:
	@$(call toc_make_subdirs,$(SUBDIRS),test)

.PHONY: test-. test-subdirs
test-.:
test: test-. test-subdirs
	@true
