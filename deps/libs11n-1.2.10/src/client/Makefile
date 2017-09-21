include toc.make

SUBDIRS = s11nconvert

ifneq (,$(strip $(filter dist testdist clean distclean,$(MAKECMDGOALS))))
      # only build these SUBDIRS when running certain targets
      SUBDIRS += sample
endif

all: subdirs
