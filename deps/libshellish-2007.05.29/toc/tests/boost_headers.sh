# toc_run_description = looking for boost headers
# toc_begin_help =
#
#  Looks for headers for the boost C++ libraries (www.boost.org).
#
# Requires --with-boost=/path/to/boost or one of the environment vars
# ${configure_with_boost} or ${BOOST_HOME}, which must point to the
# top-most directory of the boost tree (e.g., the boost source
# distribution tree). If this is not set then it checks some common
# paths for Boost. For example, if boost lives under
# /usr/dev/include/boost, you should set BOOST_HOME=/usr/dev/include
# or pass in --with-boost=/usr/dev/include.
#
# It does not check for boost code which requires compilation, only
# some common headers files.
#
# It calls toc_export for these config vars: 
#
# - HAVE_BOOST_HEADERS = zero or one
# - BOOST_HOME = directory ABOVE the boost headers dir, or empty if
#                HAVE_BOOST_HEADERS is zero.
#
# It accepts an optional list of boost headers to check, which should
# be given as relative paths, assuming a top path of ${BOOST_HOME}.
#
# = toc_end_help

if test x1 = "x${configure_with_boost}"; then
    toc_export configure_with_boost=${BOOST_HOME-${prefix}/include}
fi

toc_export BOOST_HOME=
toc_export HAVE_BOOST_HEADERS=0

test x0 = "x${configure_with_boost}" && {
    echo "Boost has been explicitely disabled via --without-boost."
    return 0
}

if test x != "x${configure_with_boost}"; then
    checkp="${configure_with_boost}"
else
    checkp="${prefix}/include /usr/include /usr/local/include $HOME/include"
fi

boostpath=
for p in $checkp; do
    test -d $p/boost && {
        boostpath=$p/boost
        break
    }
done

if test "x$boostpath" = x; then
        echo "--with-boost=/path/to/boost not set or does not point to a valid boost directory, "
        echo "and boost includes dir not found in path [$checkp]."
        unset boostpath
        unset checkp
        return 1
fi

unset checkp

#echo boostpath=$boostpath
for i in ${@-version.hpp config.hpp}; do
    toc_find $i $boostpath || {
        unset boostpath
        return 1
    }
done

boostpath=${boostpath%%/boost}
toc_export HAVE_BOOST_HEADERS=1
toc_export BOOST_HOME="$boostpath"
unset boostpath

return 0
