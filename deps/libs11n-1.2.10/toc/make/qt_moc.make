# This is out-moded. Don't use it.
#
# manages moc file dependencies and compiling for Qt moc sources.
#
# Usage:
# include $(TOC_MAKESDIR)/qt_moc.make
#
# Effects:
# - moc_*.* are managed for you (deps, compiling

QT_MOC_MAKEFILE = $(TOC_MAKESDIR)/qt_moc.make

QT_MOC_DEPSFILE = .toc.QT_MOC_DEPS.make

moc_%.cpp:
	@in=$@; in=$${in%%.cpp}.h; in=$${in##moc_}; \
	cmd="$(MOC) $$in -o $@"; echo $$cmd; $$cmd

QT_MOC_RULESGEN = $(TOC_MAKESDIR)/makerules.QT_MOC_DEPS
QT_MOC_DEPS: $(QT_MOC_DEPSFILE)
$(QT_MOC_DEPSFILE): Makefile $(QT_MOC_MAKEFILE) $(QT_MOC_RULESGEN)
	@echo "Generating moc file dependencies..."; \
		$(QT_MOC_RULESGEN) > $@

include $(QT_MOC_DEPSFILE)

deps: QT_MOC_DEPS
mocs: QT_MOC_DEPS $(QT_MOC_OUT_SOURCES)


all: QT_MOC_DEPS
