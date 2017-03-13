# Makefile snippet to create symlinks to
# remote files in the current dir.
#
# sample usage:
#   default: all
#   SYMLINKS = $(wildcard somedir/*.h)
#   include path/to/this_file
#   all: links 

$(SYMLINKS):
	@if test -e $@ -a ! -e `basename $@`; then \
		link="ln -s $@ ."; \
		echo $$link; \
		test -L $@ && continue; \
		test -f $@ && $$link || echo "link failed!"; \
	fi
.PHONY: links $(SYMLINKS)

# todo: figure out how to do this in the context of the qmake-driven build:
#ifneq($(USE_PCH),'')
#	@if test ! -L $(notdir $@.pch); then \
#		link="ln -s $@.pch ."; \
#		echo $$link; \
###		test -L $@.pch && continue; \
###		test -f $@.pch && $$link || echo "link failed!"; \
#		$$link || echo "link failed!"; \
#	fi
#endif


links: $(SYMLINKS)

clean: cleanlinks

cleanlinks:
	@cleanfiles= ; \
	for f in $(notdir $(SYMLINKS)); do \
		if test -L $$f; then \
			cleanfiles="$$cleanfiles $$f"; \
		fi; \
		if test -L $$f.pch; then \
			cleanfiles="$$cleanfiles $$f.pch"; \
		fi; \
	done; \
	if test -n "$$cleanfiles"; then \
		echo rm $$cleanfiles; \
		rm $$cleanfiles; \
	fi
