# toc_run_description = search for GNU find


toc_find find || return 1
app=${TOC_FIND_RESULT}
"$app" --version | grep -i GNU > /dev/null || {
        echo "Your 'find' ($app) appears to be non-GNU."
        return 1
}
toc_add_make FIND=$app
return 0

