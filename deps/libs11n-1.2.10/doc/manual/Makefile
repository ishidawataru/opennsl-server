#!/usr/bin/make -f
include toc.make

MAN_ODT = libs11n.odt
$(MAN_ODT):

MAN_PDF = $(patsubst %.odt,%.pdf,$(MAN_ODT))
INSTALL_DOCS = $(MAN_ODT) $(wildcard $(MAN_PDF))
INSTALL_DOCS_DEST = $(prefix)/share/doc/$(PACKAGE_NAME)/$(PACKAGE_VERSION)

DIST_FILES += $(MAN_ODT)

$(MAN_PDF): $(MAN_ODT)
	@test $@ -nt $(MAN_ODT) || \
	{ echo "Go generate the PDF!"; exit 127; }


MAN_DISTNAME = libs11n-manual-$(PACKAGE_VERSION)
TARBALL = $(MAN_DISTNAME).tar.bz2
$(TARBALL): Makefile $(MAN_ODT) $(MAN_PDF)
	@files=""; \
	for i in libs11n.{odt,pdf}; do \
		tgt=$(MAN_DISTNAME).$${i##*.}; \
		files="$$files $$tgt"; \
		cp -vp $$i $$tgt; \
	done; \
	tar cjf $@ $$files || exit; \
	ls -lat $@

CLEAN_FILES += $(wildcard $(MAN_DISTNAME)* *.pdf *.xhtml)
tar: $(TARBALL)

all:
