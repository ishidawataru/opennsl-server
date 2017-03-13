#!/do/not/bash
# toc_run_description = looking for libs11n-config
#
# toc_begin_help =
# Looks for libs11n (available from s11n.net). Use --s11n-prefix
# to set the prefix under which s11n should be found. e.g.:
#
#   ./configure --s11n-prefix=/usr/local
#
# Optionally, pass the prefix to this test as $1. The default prefix is ${prefix}.
#
# Only works with libs11n 0.9 or higher.
#
# This test exports the following config vars:
#
# - HAVE_LIBS11N = 0 or 1
#
# - LIBS11N_PREFIX: an empty value if the test fails or s11n's
# installation prefix if the test passes.
#
# - LIBS11N_CLIENT_LDADD: an empty value or:
#	-L${LIBS11N_PREFIX}/lib -ls11n [possibly also -lz -lbz]
#	(This should arguably contain -export-dynamic, but clients are expected
#	to use that, anyway.)
#
# - LIBS11N_CLIENT_INCLUDES: an INCLUDES-compatible entry, like -I${LIBS11N_PREFIX}/include
#
# - LIBS11N_LIBRARY_VERSION: the string which is returned by s11n::library_version().
#
# - LIBS11N_CONFIG=/path/to/libs11n-config
#
# = toc_end_help


toc_export HAVE_LIBS11N=0
if test x0 = "x${configure_with_s11n-1}"; then
    echo "s11n was explicitely disabled."
    return 0
fi

s11n_path=${1-${s11n_prefix-${prefix}}}:${prefix}/bin:${PATH}

toc_find libs11n-config $s11n_path  || {
    echo "libs11n-config not found in [$s11n_path]."
    unset s11n_path
    return 1
}


s11nconfig=${TOC_FIND_RESULT}
toc_export HAVE_LIBS11N=1
toc_export LIBS11N_PREFIX=$($s11nconfig --prefix)
toc_export LIBS11N_CONFIG=$s11nconfig

eval $($s11nconfig --toc-config | sed -e 's/$/; /')
unset s11nconfig

return 0


