# Use like this:
#  MKDIRS = list of dirs
#  include $(top_makefilesdir)/mkdirs.make
#  mytarget: $(MKDIRS)
#
# That's it! That's look for those dirs, create them if they
# don't exist, and exit if it can't make one.

mkdir=mkdir
## mkdir=mkdirhier

$(MKDIRS):
	@test -d $@ || $(mkdir) $@ && exit 0; \
		echo "Failed to create dir [$@]!"; \
		exit 1;
