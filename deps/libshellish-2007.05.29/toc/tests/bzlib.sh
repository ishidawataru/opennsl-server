# toc_run_description = looking for bzip lib and header
# toc_begin_help =
# Searches for libbz2.a and bzlib.h in commonly-used locations.
# It calls toc_export for the following vars:
#
# - HAVE_BZ2LIB = 0 or 1
# - LDADD_BZ2LIB = empty or -lz
#
# Pass --disable-bzlib to configure to explicitely set
# HAVE_BZLIB to zero.
#
# It returns zero if HAVE_BZLIB==1, else non-zero.
# = toc_end_help

toc_export HAVE_BZLIB=0
toc_export LDADD_BZLIB=

test x0 = x${configure_enable_bzlib} && {
    echo "bzlib explicitely disabled via --disable-bzlib."
    return 0
}

toc_find libbz2.a ${prefix}/lib:/usr/lib:/usr/local/lib:${LD_LIBRARY_PATH}
zlib_a=${TOC_FIND_RESULT}

toc_find bzlib.h ${prefix}/include:/usr/include:/usr/local/include
zlib_h=${TOC_FIND_RESULT}

test "x" = "x$zlib_a" -o "x" = "x$zlib_h"
HAVE_BZLIB=$?

if [ x1 = x${HAVE_BZLIB} ]; then
    toc_export HAVE_BZLIB=1
    toc_export LDADD_BZLIB=-lbz2
    return 0
fi
return 1
