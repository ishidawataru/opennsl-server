# toc_run_description = building under cygwin?

test -d /cygdrive/c
err=$?
if test $err = 0 ; then
    toc_add_config CYGWIN=1
    echo "Detected cygwin."
else
    toc_add_config CYGWIN=0
fi
return $err


