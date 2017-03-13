#!/usr/bin/make -f
# sample usage:
#
#   default: all
#   QMAKES = foo bar # requires that {foo,bar}.qmake exist
#   include path/to/qmake2make.make
#   all: qmake
#
# It generates and runs make.X for each X.qmake, where X is an entry
# in $(QMAKES). It also provides these global targets:
#  qmake-X-{clean,distclean,install,uninstall,all} qmake-X
#
# It also modifies the deps for clean, distclean, install and uninstall
# to call the appropriate targets in the qmake-processed files.


QMAKE_QMAKE2MAKE_MAKE = $(TOC_MAKESDIR)/qmake2make.make
ifeq (,$(wildcard $(QTDIR)))
$(error $(QMAKE_QMAKE2MAKE_MAKE) requires the QTDIR var. Maybe you need to run the qt test?)
endif

ifeq (,$(TOC_MKDEP_BIN))
include $(TOC_MAKESDIR)/C_DEPS.make
endif

MAKE_NO_PRINT_DIR = ${MAKE} --no-print-directory

QMAKEFILE_SUFFIX = .qmake
GNUMAKEFILE_PREFIX = make.

QMAKEFILES = $(patsubst %,%$(QMAKEFILE_SUFFIX),$(QMAKES))
QMAKED_MAKEFILES = $(patsubst %,$(GNUMAKEFILE_PREFIX)%,$(QMAKES))
# note the makefile naming inconsistency: foo.qmake -> make.foo
# this is to avoid [me having] confusion when looking at the filenames,
# and to prevent accidental deletion via a 'rm *.qmake' typo.
# It also helps solve tab-completion problems ;).

.PHONY: $(QMAKES)

$(QMAKEFILES):
	@true

$(GNUMAKEFILE_PREFIX)%: %$(QMAKEFILE_SUFFIX) Makefile $(QMAKE_QMAKE2MAKE_MAKE)
	@test -f "$<" || exit 0; \
	echo -n "Creating $@: "; \
	cmd="$(QMAKE) -o $@ $*$(QMAKEFILE_SUFFIX)"; echo $$cmd; $$cmd

#ifeq (,$(strip $(filter clean distclean,$(MAKECMDGOALS))))
# we can't ignore clean because the qmake file may need to be regen'd to change it's clean rules. :/
#endif
# todo: some more complete rules, gen'd via makerules.QMAKES, and then add only an
# 'include ( ... )' to the qmake output.

CLEAN_FILES += $(QMAKED_MAKEFILES)

deps: $(QMAKED_MAKEFILES)

# create targets for qmake-MAKEFILE-TARGET:
# QMAKE_TARGETS_TO_PROPOGATE = first all install uninstall clean distclean staticlib mocables

# simplified version, for compatibility with make 3.79.1 :(
# note that i cannot make it depend on make.% because then it fucks up, refusing to accept targets
# in the form qmake-X-Y.
# i HATE that i can't use the $(eval) code, but make 3.79 (without
# $(eval)) is common and it needs to be supported.
qmake-%:# expects qmake-MAKEFILE_BASE_NAME-TARGET, e.g. qmake-myproject-all
	@t=$*; head=$${t%%-*}; tail=$${t##*-}; \
	mf=$(GNUMAKEFILE_PREFIX)$$head; \
	test "$$head" = "$$tail" && tail=all; \
	echo "qmake2make: Making target '$$tail' in $$mf..."; \
	${MAKE} -f $$mf $$tail;

#	${MAKE} -s $$mf;



# tie in to the standard targets:
install: $(patsubst %,qmake-%-install,$(QMAKES))
uninstall: $(patsubst %,qmake-%-uninstall,$(QMAKES))
first: $(patsubst %,qmake-%-first,$(QMAKES))
clean: $(patsubst %,qmake-%-clean,$(QMAKES))
distclean: $(patsubst %,qmake-%-distclean,$(QMAKES))
staticlib: $(patsubst %,qmake-%-staticlib,$(QMAKES))
mocables: $(patsubst %,qmake-%-mocables,$(QMAKES))


qmake: $(patsubst %,qmake-%,$(QMAKES))

DISTCLEAN_FILES += $(QMAKED_MAKEFILES)
# ^^^ these /should/ be CLEAN_FILES, but that causes a lot of unneeded file rebuilding.
