# toc_run_description = looking for libltdl
# toc_begin_help =
#
# Looks for libltdl. It calls toc_export
# for the following variables:
#
#	HAVE_LIBLTDL = 0 or 1
#	LDADD_LIBLTDL = -lltdl or empty
#	LDADD_DL = empty or "-ltdl -rdynamic", possibly with -L/path...
#
# Note that LDADD_LIBLTDL is specific to the libltdl test, whereas LDADD_DL is used
# by both the libdl and libltdl tests.
#
# = toc_end_help
# Many thanks to Roger Leigh for introducing me to ltdl!

toc_add_config HAVE_LIBLTDL=0
toc_add_config LDADD_LIBLTDL=
toc_export LDADD_DL=

err=1

if toc_test gcc_build_and_run ${TOC_HOME}/tests/c/check_for_ltdlopen_and_friends.c -rdynamic -lltdl; then
    err=0
    toc_export HAVE_LIBLTDL=1
    toc_export LDADD_LIBLTDL="-lltdl -rdynamic"
    toc_export LDADD_DL="${LDADD_LIBLTDL}"
fi

return $err



