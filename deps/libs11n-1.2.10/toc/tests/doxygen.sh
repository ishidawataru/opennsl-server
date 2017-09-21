#!/do/not/bash
# toc_run_description = looking for doxygen, API doc generation tool.
# toc_begin_help = 
#
#  Looks for doxygen in the ${PATH} or the binary specified using
#  --with-doxygen=/path/to/doxygen.  It calls toc_export for the
#  following vars:
#
#  - DOXYGEN_BIN=/path/to/doxygen, or empty string if we got
#	--without-doxygen or a bad value for --with-doxygen=...
#
# It returns zero if it finds doxygen, else non-zero.
#
# = toc_end_help

if test "x${configure_with_doxygen}" = "x0"; then
    echo "doxygen support has been explicitely disabled."
    toc_export DOXYGEN_BIN=
    return 1
fi

if test "x${configure_with_doxygen}" = x || test "x${configure_with_doxygen}" = x1; then
    toc_find_in_path doxygen "$PATH" && DOXYGEN_BIN=${TOC_FIND_RESULT}
elif test "x${configure_with_doxygen}" = x0; then
    DOXYGEN_BIN=
else
    if test -x ${configure_with_doxygen}; then
        DOXYGEN_BIN="${configure_with_doxygen}"
    else
    #  I hate shell scripts!!
        echo '--with-doxygen "'"${configure_with_doxygen}"'" isn'"'t executable!"
        toc_export DOXYGEN_BIN=
        return 1
    fi
fi
toc_export DOXYGEN_BIN="$DOXYGEN_BIN"
return 0
