include toc.make

SUBDIRS = sbin bin tests make

# We don't normally want to include toc's docs in non-toc trees:
ifeq (toc,$(PACKAGE_NAME))
  SUBDIRS += doc
endif

all: subdirs
