#!/do/not/make
# ^^^ help out emacs
# Makefile snippet to traverse subdirs.
# Sample usage:
#  default: all
#  SUBDIRS = foo bar
#  include path/to/subdirs_traverser.make
#  all: subdirs
# NOTES:
#  - SUBDIRS must be defined BEFORE including this file. This is a
# side-effect of the implementation: make expands this var
# at compile time, so to say, so it can build the proper subdir
# targets. Adding to the SUBDIRS var later doesn't affect which targets
# get created, but ARE necessary for targets like install-subdirs :/.
#
# DO NOT put "." in the SUBDIRS! Instead, client Makefiles completely
# control dir build order via dependencies.


# toc_make_subdirs call()able function:
# $1 = list of dirs
# $2 = target
ifeq (1,$(configure_build_quietly))
MAKE_NOP_ARG = --no-print-directory
endif
toc_make_subdirs = \
	test "x$(1)" = "x" && exit 0; \
	tgt="$(2)"; test x = "x$$tgt" && tgt="all"; \
	for b in $(1) "x"; do test "x" = "$$b" && break; \
		pwd=$$(pwd); \
		echo "Making $$tgt in $${b}"; \
		${MAKE} -C $${b} $(MAKE_NOP_ARG) $$tgt || exit; \
		cd $$pwd || exit; \
	done


.PHONY: subdirs	distclean-subdirs clean-subdirs $(SUBDIRS)

# note that this only works for subdirs defined BEFORE including this file:
$(SUBDIRS): FORCE
	${MAKE} -C $@
#	@$(call toc_make_subdirs,$@)


subdir-%: # make $* as a subdirectory
	@test "x$*" = "x." && exit 0; $(call toc_make_subdirs,$*)

subdirs-%: ## calls $* target in all $(SUBDIRS)
	@$(call toc_make_subdirs,$(SUBDIRS),$*)

#subdirs: $(addprefix subdir-,$(SUBDIRS))
subdirs: subdirs-all
#	@${MAKE} $(addprefix subdir-,$(SUBDIRS))
#subdirs-all
#	@$(call toc_make_subdirs,$(SUBDIRS),all)



