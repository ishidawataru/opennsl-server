#!/do/not/bash
# ^^^ help out emacs
#
# toc_run_description = use ccdv compiler front-end?
#
# toc_begin_help =
#
# Running this test enabled Mike Gleason's ccdv compiler front-end,
# which simply pretties-up compiler output. The ccdv source lives in
# ${TOC_HOME}/bin/ccdv.c and is released under the GNU GPL.
#
# Use the --disable-ccdv configure option to explicitely disable it.
#
# = toc_end_help

test 0 = "${configure_enable_ccdv-1}" && {
    echo "ccdv has been explicitely disabled with --disable-ccdv."
    return 0
}

test x != "x${CC}" || {
    echo 'This test requires that the CC environment variable be set to a C compiler!'
    return 1
}

ccdvsrc=${TOC_HOME}/bin/ccdv.c

toc_find $ccdvsrc || {
    unset ccdvsrc
    return 1
}


${CC} -o ccdv ${ccdvsrc} || {
    unset ccdvsrc
    echo "Compilation of ccdv failed!"
    return 1
}

unset ccdvsrc

return 0
