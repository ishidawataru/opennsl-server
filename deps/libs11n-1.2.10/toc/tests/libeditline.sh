# toc_run_description = looking for liblineedit
# toc_begin_help =
# Searches for liblineedit, a BSD-licensed replacement for GNU libreadline.
#
#Use --without-libeditline to explicitely disable libeditline support.
#
#toc_export is called for the following variables:
#
#	HAVE_EDITLINE = 0 or 1
#
#	LIBEDITLINE_LDADD = empty or -leditline, possibly with -lncurses
#
# = toc_end_help

toc_export HAVE_EDITLINE=0
toc_export LIBEDITLINE_LDADD=

configure_with_libeditline=${configure_with_libeditline-1}
test "x${configure_with_libeditline}" = "x0" && {
    echo "libeditline has been explictely disabled with --without-libeditline."
    return 0
}


_libpath=/usr/lib:/usr/local/lib:${prefix}/lib:/lib
_do=0
for L in libeditline.so libeditline.a; do
    toc_find $L "$_libpath" && {
        _do=1
        break
    }
done
test x$_do = x0 && {
    echo "libeditline library file not found."
    unset _do
    unset _libpath
    return 0
}
rllib=${TOC_FIND_RESULT}

rlincpath=/usr/include:/usr/include/editline:/usr/local/include:/usr/local/include/editline:${prefix}/include:${prefix}/include/editline
for inc in editline.h; do
    toc_find $inc $rlincpath || {
        err=$?
        echo "Header file $inc not found in path [$rlincpath]"
        unset rlincpath
        unset rllib
        return $err
    }
done
unset rlincpath

LIBEDITLINE_LDADD="-leditline"

# odd: lineedit sources never included curses.h, but do use curses functions,
# leading to an unknown symbol in your app if we don't tell you that you need
# to link against curses:
#if ldd $rllib 2> /dev/null | grep ncurses &>/dev/null ; then
#    echo "It seems that libeditline also requires linking against -lncurses."
LIBEDITLINE_LDADD="${LIBEDITLINE_LDADD} -lncurses"
#fi

toc_export HAVE_EDITLINE=1
toc_export LIBEDITLINE_LDADD="${LIBEDITLINE_LDADD}"
unset nc
return 0
