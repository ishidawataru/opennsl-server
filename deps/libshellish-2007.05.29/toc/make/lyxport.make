#!/usr/bin/make -f
# Don't use this:
# a) it requires GNU Make 3.80 or newer (to use $(eval)).
# b) it needs to be rewritten, in any case.
#
# usage:
# define:
#  LXY_FILES = list of lyx files
#
# all lyx-pdf lyx-html lyx-ps

LYX2X_MAKEFILE = $(TOC_MAKESDIR)/lyxport.make
ifeq (,$(wildcard $(LYXPORT_BIN)))
$(error this makefile requires LYXPORT_BIN. Try running the lyxport test.)
endif


LYXPORT_ARGS ?= -tt

LYX_FILES_BASENAMES = $(basename $(LYX_FILES))

define LYX2X_TARGET_LYX
$(1).lyx: Makefile $$(LYX2X_MAKEFILE)
endef
$(foreach foo,$(LYX_FILES_BASENAMES),$(eval $(call LYX2X_TARGET_LYX,$(foo))))

define LYX2X_TARGET_LYXPORT_LYX
CLEAN_FILES += $(1).ps $(1).pdf $(1).html $(1).latex
lyxport-$(1): $(1).lyx;\
	$$(LYXPORT_BIN) $$(LYXPORT_ARGS) $$<
endef
$(foreach foo,$(LYX_FILES_BASENAMES),$(eval $(call LYX2X_TARGET_LYXPORT_LYX,$(foo))))


define LYX2X_TARGET_STUBS
lyxport: lyxport-$(1)
endef
$(foreach foo,$(LYX_FILES_BASENAMES),$(eval $(call LYX2X_TARGET_STUBS,$(foo))))

##define LYX2X_TARGET_ALL_DEPS
##all: $(1)
