#!/do/not/make
#
# snippet to symlink header files to a common includes dir.
#
# Usage:
#   SYMLINK_HEADERS = $(wildcard *.h)
#   SYMLINK_HEADERS_DEST = $(top_srcdir)/include/whereever
#   include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make
# run:
#  all: SYMLINK_HEADERS
#
# The headers are cleaned up during clean/distclean.

ifneq (,$(SYMLINK_HEADERS))
ifeq (,$(SYMLINK_HEADERS_DEST))
$(error You must define both SYMLINK_HEADERS and SYMLINK_HEADERS_DEST to use this snippet.)
endif

symlink-headers: FORCE
	@$(call toc_make_install_symlink,$(SYMLINK_HEADERS),$(SYMLINK_HEADERS_DEST),-m 0644)

SYMLINK_HEADERS: symlink-headers

symlink-headers-clean: FORCE
	@echo "Cleaning headers."
	@-touch foo.cleanlocal; rm foo.cleanlocal $(wildcard $(addprefix $(SYMLINK_HEADERS_DEST)/,$(SYMLINK_HEADERS)))

SYMLINK_HEADERS_CLEAN: symlink-headers-clean

clean: symlink-headers-clean
distclean: symlink-headers-clean

endif
