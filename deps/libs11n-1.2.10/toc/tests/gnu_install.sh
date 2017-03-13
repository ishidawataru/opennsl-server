# tests for GNU install
toc_add_make PACKAGE_IS_INSTALLABLE=0

toc_find ginstall || toc_find install  || {
        echo "install or ginstall not found in your PATH: $PATH"
        return 1
}
app=${TOC_FIND_RESULT}
"$app" --help |  2>&1 grep -i GNU > /dev/null || {
    echo "Your 'install' ($app) appears to be non-GNU."
    return 1
}

toc_add_make GNUINSTALL_BIN=$app
toc_add_make PACKAGE_IS_INSTALLABLE=1

return 0

