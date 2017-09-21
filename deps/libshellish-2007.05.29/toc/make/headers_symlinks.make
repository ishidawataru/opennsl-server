# Makefile snippet to create symlinks.
#
# sample usage:
#   default: all
#   HEADERS_SYMLINKS = $(wildcard somedir/*.h)
#   include path/to/this_file
#   all: links 

$(HEADERS_SYMLINKS):
	@test -e `basename $@` && exit 0; \
	link="ln -s $@ ."; \
	echo $$link; \
	$$link || echo "link failed!"

.PHONY: headers_links $(HEADERS_SYMLINKS)


headers_symlinks: $(HEADERS_SYMLINKS)

