#!/do/not/bash
# toc_run_description = checks for --help-X and tries to do something about it.
# toc_begin_help =
#
# toc_tests_help looks for vars named help_* and tries to find help text for
# tests matching that pattern (that is, help_foo looks for help for test foo).
# To add help to your test wrap it in a block which begins with
# the text 'toc_begin_help =' and end it with '= toc_end_help'
#
# This test returns zero only if no --help-X options are specified.
#
# The --help-TESTNAME options do not properly work with tests which:
#
# - do not live directly in toc/tests
# - contain minus signs in their names
#
# The workaround is to use --help-tests, which will provide you with a selection
# list of all tests under toc/tests and it's subdirectories.
#
# = toc_end_help

toc_show_test_help ()
{
    local scr=${1?first argument must be a filename}
    test -f $scr || toc_die 127 "Test $scr not found!"
    sed -n '/^# *toc_begin_help *=/,/^#\ *= *toc_end_help/p' $scr | \
        sed '/^# *toc_begin_help *=/d;/^# *= *toc_end_help/d;s/^# */    /;'
}


toc_select_test_help ()
{
    local tests=$(cd ${TOC_HOME}/tests; find . -type f -name '*.sh' | xargs grep -l toc_begin_help | sort | sed -e "s,\./,,;s,\.sh,,;")
    PS3="Select a test to show it's help: "
    select T in $tests; do
        echo "Help for '$T' test:"
        toc_show_test_help ${TOC_HOME}/tests/${T}.sh
        break
        # i'd like to loop, but the selection list is only shown on the first run,
        # and quickly scrolls off the screen, making a 'break' the only usable option.
    done
}


if test x1 = x${help_tests} ; then
    toc_select_test_help
    return 1
fi

helps="$(env | grep '^help_')"
test -n "$helps" || return 0
for h in $helps; do
    test=${h##help_}
    test=${test%%=*}
    echo "help: $test"
    toc_find_test $test || {
        echo "Error: cannot find test $test."
        return 1
    }
    scr=${TOC_FIND_RESULT}
    grep -q "toc_begin_help" $scr || {
        echo "No help text found."
        continue
    }
    toc_show_test_help $scr
done

return 1
