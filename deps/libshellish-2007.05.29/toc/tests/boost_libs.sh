# toc_run_description = looking for boost libraries
# toc_begin_help =
#
#  Looks for library files for the boost C++ libraries (www.boost.org).
#
# If the Boost libs are in a non-standard place, it requires
# --with-boost-libs=/path/to/boost or the environment var
# ${configure_with_boost_libs}, which must point to the dir where
# the library files live. If this is not set then it checks some common paths for
# Boost. For example, if boost lives under /usr/dev/lib, you
# should pass in --with-boost-libs=/usr/dev/lib
#
# It calls toc_export for these config vars: 
#
# - HAVE_BOOST_LIBS = zero or one
# - BOOST_LIBS_HOME = /path/to/libs/dir
#
# It accepts an optional list of boost libs to look for, which should
# be given in this form: libboost_XXXX.so is entered as XXXX. The libraries
# are assumed to live in ${BOOST_HOME}/lib
#
# = toc_end_help

if test x1 = "x${configure_with_boost_libs}"; then
    toc_export configure_with_boost_libs=${BOOST_LIBS_HOME-${prefix}/lib}
fi

toc_export BOOST_LIBS_HOME=
toc_export HAVE_BOOST_LIBS=0

test x0 = "x${configure_with_boost_libs}" && {
    echo "Boost has been explicitely disabled via --without-boost."
    return 0
}

if test x != "x${configure_with_boost_libs}"; then
    checkp="${configure_with_boost_libs}"
else
    checkp="${prefix}/lib /usr/lib /usr/local/lib ${HOME}/lib"
fi

boostpath=
for p in $checkp; do
    ls $p/libboost* &> /dev/null && {
        boostpath=$p
        break
    }
done

if test "x$boostpath" = x; then
        echo "--with-boost-libs=/path/to/boost/libs not set or does not point to a directory containing 'libboost*'."
        unset boostpath
        unset checkp
        return 1
fi

unset checkp

for i in ${@-filesystem regex signals}; do
    soname="libboost_${i}.so" # evil, hard-coded ".so"
#    echo -n "? ${soname} ..."
#    test -e ${soname} || {
    toc_find "$soname" "${boostpath}" || {
        unset boostpath
	toc_boldecho "Boost Library '${soname}' not found"
        return 1
    }
done

boostpath=${boostpath%%/boost}
toc_export HAVE_BOOST_LIBS=1
toc_export BOOST_LIBS_HOME="$boostpath"
unset boostpath

return 0
