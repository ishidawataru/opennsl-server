# toc_run_description = looking for zlib
# toc_begin_help =
# Searches for libz.a and zlib.h in commonly-used locations.
# It calls toc_export for the following vars:
#
# - HAVE_ZLIB = 0 or 1
# - LDADD_ZLIB = empty or -lz
#
# Pass --disable-zlib to configure to explicitely set
# HAVE_ZLIB to zero.
#
# It returns zero if HAVE_ZLIB==1, else non-zero.
# = toc_end_help

toc_export HAVE_ZLIB=0
toc_export LDADD_ZLIB=
test x0 = x${configure_enable_zlib} && {
    echo "zlib explicitely disabled via --disable-zlib."
    return 0
}

toc_find libz.a ${prefix}/lib:/usr/lib:/usr/local/lib:${LD_LIBRARY_PATH}
zlib_a=${TOC_FIND_RESULT}

toc_find zlib.h ${prefix}/include:/usr/include:/usr/local/include
zlib_h=${TOC_FIND_RESULT}

test "x" = "x$zlib_a" -o "x" = "x$zlib_h"
HAVE_ZLIB=$?

if [ x1 = x${HAVE_ZLIB} ]; then
    toc_export HAVE_ZLIB=1
    toc_export LDADD_ZLIB=-lz
    return 0
fi
return 1
