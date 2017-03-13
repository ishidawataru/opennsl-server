# This is out-moded. Don't use it.
#
# manages moc file dependencies and compiling for Qt moc sources.
#
# Usage:
# include $(TOC_MAKESDIR)/qt_moc.make
#
# Effects:
# - $(MOC_PREFIX)*.* are managed for you (deps, compiling

MOC_PREFIX = moc_
QT_MOC_MAKEFILE = $(TOC_MAKESDIR)/qt_moc.make

QT_MOC_DEPSFILE = .toc.QT_MOC_DEPS.make

$(MOC_PREFIX)%.cpp:
	@in=$@; in=$${in%%.cpp}.h; in=$${in##$(MOC_PREFIX)}; \
	cmd="$(MOC) $$in -o $@"; echo $$cmd; $$cmd

QT_MOC_RULESGEN = $(TOC_MAKESDIR)/makerules.QT_MOC_DEPS
QT_MOC_DEPS: $(QT_MOC_DEPSFILE)
$(QT_MOC_DEPSFILE): Makefile $(wildcard *.h *.hpp *.hxx) $(QT_MOC_RULESGEN)
ifeq (1,$(MAKING_CLEAN))
	@echo "$(MAKECMDGOALS): skipping moc file dependencies generation."
else
	@echo "Generating moc file dependencies..."; \
		$(QT_MOC_RULESGEN) $(MOC_PREFIX) > $@
endif

-include $(QT_MOC_DEPSFILE)

deps: QT_MOC_DEPS
mocs: QT_MOC_DEPS $(QT_MOC_OUT_SOURCES)


all: QT_MOC_DEPS
