# toc_run_description = looking for GNU libreadline/libhistory
# toc_begin_help =
# Searches for GNU libreadline.
#
#Use --without-libreadline to explicitely disable libreadline support.
#
#toc_export is called for the following variables:
#
#	HAVE_READLINE = 0 or 1
#
#	LIBREADLINE_LDADD = empty or -lreadline, possibly with -lncurses
#
# = toc_end_help

toc_export HAVE_READLINE=0
toc_export LIBREADLINE_LDADD=

configure_with_libreadline=${configure_with_libreadline-1}
test "x${configure_with_libreadline}" = "x0" && {
    echo "libreadline has been explictely disabled with --without-libreadline."
    return 0
}


_libpath=/usr/lib:/usr/local/lib:${prefix}/lib:/lib
_do=0
for L in libreadline.so libreadline.so.4 libreadline.a; do
    toc_find $L "$_libpath" && {
        _do=1
        break
    }
done
test x$_do = x0 && {
    echo "libreadline library file not found."
    unset _do
    unset _libpath
    return 0
}
rllib=${TOC_FIND_RESULT}

rlincpath=/usr/include:/usr/include/readline:/usr/local/include:/usr/local/include/readline:${prefix}/include:${prefix}/include/readline
for inc in readline.h history.h; do
    toc_find $inc $rlincpath || {
        err=$?
        echo "Header file $inc not found in path [$rlincpath]"
        unset rlincpath
        unset rllib
        return $err
    }
done
unset rlincpath

LIBREADLINE_LDADD="-lreadline"

if ldd $rllib 2> /dev/null | grep ncurses &>/dev/null ; then
    echo "It seems that libreadline also requires linking against -lncurses."
    LIBREADLINE_LDADD="${LIBREADLINE_LDADD} -lncurses"
fi

toc_export HAVE_READLINE=1
toc_export LIBREADLINE_LDADD="${LIBREADLINE_LDADD}"
unset nc
return 0
