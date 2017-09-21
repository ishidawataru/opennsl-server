# This file is sourced by configure. Do not run it standalone:
# it is designed to be sources from the toconfigure script.
#
# It contains the implementation of the toc API.
#
#
# All global vars defined in this file are, unless otherwise noted,
# considered private - configure is not supposed to touch them
# directly, nor are test scripts.
#
# Sourcing this file from anywhere other than toconfigure will have undefined
# results.
#

########################################################################
# maintenance notes:
#
# vars expected from toconfigure:
# TOC_HOME - path to toc's top dir
# TOP_SRCDIR - directory we want to configure from (e.g., top-most
#    dir of a source tree).

TOC_QUIET=${TOC_QUIET-0}
TOC_LOUD=${TOC_LOUD-0}

export prefix=${prefix-${TOC_PREFIX-/usr/local}}
# prefix will be overridden by the command-line arg --prefix=/path
TOC_INCLUDESDIR=${TOP_SRCDIR}/include
TOC_TESTSDIR=${TOC_HOME}/tests
TOC_MAKEFILES_DIR=$TOC_HOME/make

TOC_ATSIGN_PARSER=$TOC_HOME/bin/atsign_parse # utility to replace @TOKENS_LIKE_THIS_ONE@ in streams.

TOC_DEBUG=${TOC_DEBUG-0}

TOC_SHARED_MAKEFILE=toc_shared.make
# TOC_SHARED_MAKEFILE is obsolete, and should Go Away. Use toc.${PACKAGE_NAME}.make.at
# and ${TOC_HOME}/make/toc.make.at instead.

declare -a CONFIG_DOT_MAKE_ARRAY # array holds key=val strings for makefiles
declare -a CONFIG_DOT_H_ARRAY # array holds key=val strings for config.h.at

TOC_DELETE_TEMP="test 1 = 1" # set to a false condition to disable deletion of some temp files (for debuggering)

TOC_TIMESTAMP=${TOP_SRCDIR}/.toc.configtimestamp
touch ${TOC_TIMESTAMP}


# TOC_VERBOSE_WARNINGS=${TOC_VERBOSE_WARNINGS-0} # spits out the debug file contents

#TOC_CACHE=${TOP_SRCDIR}/configure.last
#test -f ${TOC_CACHE} && rm ${TOC_CACHE}
# echo $0 $@ >> ${TOC_CACHE}# loses quoted args' quotes

{ # eye candy, mostly stolen from mod_ssl configure script:
    which awk >/dev/null && {
        case $TERM in
            xterm|xterm*|vt220|vt220*)
                TOC_BOLD_=`echo dummy | awk '{ printf("%c%c%c%c", 27, 91, 49, 109); }'` # that is frigging clever, no?
                _TOC_BOLD=`echo dummy | awk '{ printf("%c%c%c", 27, 91, 109); }'`
                ;;
            vt100|vt100*)
                TOC_BOLD_=`echo dummy | awk '{ printf("%c%c%c%c%c%c", 27, 91, 49, 109, 0, 0); }'`
                _TOC_BOLD=`echo dummy | awk '{ printf("%c%c%c%c%c", 27, 91, 109, 0, 0); }'`
                ;;
            default)
                TOC_BOLD_=''
                _TOC_BOLD=''
                ;;
        esac
    }

    _TOC_BOLD=${_TOC_BOLD-""}
    TOC_BOLD_=${TOC_BOLD_-""}
    toc_boldecho ()
    {
	local args=
        for i in n e; do
            if test "$1" = "-${i}"; then
                args="$args $1"
                shift
            fi
        done
        echo $args "${TOC_BOLD_}${@}${_TOC_BOLD}"
    }
    toc_boldcat ()
    {
        echo "${TOC_BOLD_}"
        cat
        echo "${_TOC_BOLD}"
    }
} # end eye candy

TOC_ERR_OK=0
TOC_ERR_ERROR=1

# OKAY, ERROR and WARNING are "core" emoticons:
TOC_EMOTICON_OKAY="${TOC_BOLD_}:-)${_TOC_BOLD}"
TOC_EMOTICON_ERROR="${TOC_BOLD_}:-(${_TOC_BOLD}"
TOC_EMOTICON_WARNING="${TOC_BOLD_}:-O${_TOC_BOLD}"

# "unofficial" emoticons:x
TOC_EMOTICON_GRIEF="${TOC_BOLD_}:'-(${_TOC_BOLD}"
TOC_EMOTICON_WTF="${TOC_BOLD_}:-?${_TOC_BOLD}"


toc_debug ()
{
    test "$TOC_DEBUG" -ne 0 || return 0;
    echo "toc_debug: " "$@" 1>&2
    return 0
}

toc_die ()
{ # $1 = error code (defaults to $?)
  # $2+ = optional informational text
  # sample: toc_test mytest || toc_die $? "mytest failed."
    err=${1-$?}
    shift
    echo -n "${TOC_EMOTICON_ERROR} "
    toc_boldecho "toc_die: exiting with error code $err."
    test -n "$1" && toc_boldecho "$@"
    exit $err
}


toc_quietly ()
{
  # outputs $@ if ${TOC_QUIET} = 0
  # returns 1 if it does not output anything
  # It it is passed no parameters it reads in from stdin,
  # thus you can:
  # toc_quietly <<EOF
  #   blah blah blah
  # EOF
    test "x${TOC_QUIET-0}" != "x0" && return 1
    test -n "$1" && {
        echo "$@"
        return 0
    }
    cat
    return 0
}

toc_loudly ()
{
  # outputs $@ if ${TOC_LOUD} != 0
  # returns 1 if it does not output anything
  # It it is passed no parameters it reads in from stdin,
  # thus you can:
  # toc_loudly <<EOF
  #   blah blah blah
  # EOF
    test "x${TOC_LOUD-0}" = "x0" && return 1
    test -n "$1" && {
        echo "$@"
        return 0
    }
    cat
    return 0
}

########################################################################
# _TOC_ATEXIT holds the path to this app's "atexit" code, which is
# executed at app exit.
_TOC_ATEXIT=$TOP_SRCDIR/.toc.atexit.sh
trap 'echo "rm $_TOC_ATEXIT" >> $_TOC_ATEXIT; $SHELL $_TOC_ATEXIT; exit;' 0 1 2 3
# ^^^^^ the 'exit' in the trap is to force a SIGINT/SIGHUP to cause an exit.
# On a trap 0 the exit is redundant yet harmless.
########################################################################
# toc_atexit() is used to provide 'trap' functionality within the TOC framework.
# Client code should NEVER use trap directly, because trap does not offer
# a way to append trap commands. i.e., if clients add a trap they will override
# the framework's trap, which won't be nice.
# $@ must be a list of commands to run at app shutdown.
# Returns 0 unless there is a usage error, in which case it returns non-zero.
# Note that complex string quoting will probably not survive the quote-conversion
# process involved in function calls and parameter passing, so if you need to
# run complex commands at exit, send them to a script and then register that
# script to be run via toc_atexit().
#
# Commands are run in the order they are registered, NOT in reverse order
# (as in C's atexit()).
#
# NEVER call exit from within atexit code!
toc_atexit()
{
    toc_debug toc_atexit "$@"
    test "x" = "x$1" && {
    echo "toc_atexit() usage error: \$@ must contain commands to run at app shutdown."
    return 1
    }
    echo "$@" >> $_TOC_ATEXIT
    return 0
}


toc_dump_make_properties ()
{ # Dumps the current properties from the makefile array to stdout
    local i=0
    while test $i -lt ${#CONFIG_DOT_MAKE_ARRAY[@]}; do
        echo ${CONFIG_DOT_MAKE_ARRAY[$i]}
        i=$((i + 1))
    done | ${TOC_HOME}/bin/cleanup_vars
    return 0
}

toc_dump_config_h_properties ()
{ # Dumps the current properties from the config.h array to stdout
    local i=0
    while test $i -lt ${#CONFIG_DOT_H_ARRAY[@]}; do
        echo ${CONFIG_DOT_H_ARRAY[$i]}
        i=$((i + 1))
    done | ${TOC_HOME}/bin/cleanup_vars
    return 0
}

toc_get_make ()
{ # gets a key which has been previously set
  # via toc_add_make. The value is not echoed, but is in the
  # global var ${TOC_GET_MAKE}. Returns 0 if it finds the var
  # and 1 if it does not.
  #
  # $1 = name of var.
  # [$2] = if 1 then the result is echoed. If 2 then $1=${TOC_GET_MAKE}
  #        is echoed. Default behaviour is to not output anything.
  #        A pneumonic for $2: 1 stands for one part (the value) and 2
  #        stands for 2 parts (the key and the value).
    TOC_GET_MAKE=
    local i=${#CONFIG_DOT_MAKE_ARRAY[@]}
    local arg="$1"
    local theval=
    local ret=1
    i=$((i - 1))
    while test $i -ge 0; do
        foo=${CONFIG_DOT_MAKE_ARRAY[$i]}
        local k="${foo%%=*}"
        local v="${foo#*=}"
        test "$arg" = "$k" && {
            ret=0
            theval="$v"
            break
        }
        i=$((i - 1))
    done
    TOC_GET_MAKE="$theval"
    test "x$2" = "x1" && echo ${TOC_GET_MAKE}
    test "x$2" = "x2" && echo $arg="${TOC_GET_MAKE}"
    toc_debug toc_get_make "$@": "$arg=$theval"
    return $ret
}

toc_atfilter_as_makefile ()
{ # filters file $1 using the vars currently set for the shared makefile.
    # $1 = template file
    # $2 = target file
    local tmpl=$1
    local target=$2
    local tmpfile=.toc.atfilter_as_makefile
    toc_dump_make_properties > $tmpfile
    toc_atfilter_file $tmpfile $tmpl $target
    local err=$?
    rm $tmpfile
    return $err
}


toc_create_make ()
{ 
    # todo: completely refactor this func, splitting it up into
    # several functions.
    #
    # Writes out toc_shared.make using the options set via toc_add_make and creates toc.make
    # echo "Creating ${TOC_SHARED_MAKEFILE} and it's friends... "

    # set some must-be-set-last vars here:
#    toc_add_make TOC_SHARED_MAKEFILE=${TOC_SHARED_MAKEFILE}
#    local toc_shared_qmakefile=${TOC_SHARED_MAKEFILE%%.make}.qmake
#    toc_add_make TOC_SHARED_QMAKEFILE=${toc_shared_qmakefile}
#    TOC_SHARED_MAKEFILE_AT=${TOC_SHARED_MAKEFILE}.at
#    toc_add_make TOC_SHARED_MAKEFILE_AT=${TOC_SHARED_MAKEFILE_AT}
#    TOC_GENDMAKE_OUT=toc.make
#    toc_add_make TOC_MAKEFILE=$TOC_GENDMAKE_OUT # so we can add this to DISTCLEAN :/

    # create toc.make for everyone who looks like they need it:
    #echo "${TOC_GENDMAKE_OUT}: "
    toc_test toc_make | toc_die $? "Error creating toc.make!"

    
    test 0 = 1 && { # .qmake support
        qtat=${toc_shared_qmakefile}.at
        test -f "$qtat" && {
            toc_atfilter_as_makefile $qtat ${qtat%%.at} || {
                toc_die $? "Error filtering $qtat."
            }

            qmakeout=toc.qmake

            #echo "$qmakeout: "
            local files=$(find ${TOP_SRCDIR} -name Makefile -o -name "*.qmake" | xargs grep -l "^include(.*$qmakeout.*)" )
            
            lastdir=
            test -n "$files" && for d in $files; do
                cmake=${d%/*}/${qmakeout}
                cmake=${cmake##$TOP_SRCDIR/}  # make it a relative path, for asthetic reasons :/
                toc_makerelative $d
                relpath=${TOC_MAKERELATIVE}
                test "$lastdir" = "$cmake" && continue
                lastdir=$cmake # keep from doing same dir twice :/
                tmpfile=${cmake}.tmp

                cp $makeprops $tocmakeprops
                cat <<EOF >> $tocmakeprops
TOC_TOP_SRCDIR=${relpath}
TOC_SHARED_QMAKEFILE=\$\$top_srcdir/${toc_shared_qmakefile}
include( \$\$TOC_SHARED_QMAKEFILE )
EOF
                 # echo -n "    $cmake ... "
                toc_atfilter_file $tocmakeprops ${TOC_HOME}/make/toc.qmake.at $cmake || {
                    err=$?
                    echo "Error creating toc.qmake!"
                    exit $err
                }
                rm $tocmakeprops # >/dev/null 2>&1
            done
            #echo ${TOC_EMOTICON_OKAY}
        } # test -f "$qtat"
    } # end qmake support

    #echo -n "${TOC_SHARED_MAKEFILE##${TOP_SRCDIR}/}: "

#    toc_atfilter_as_makefile ${TOC_SHARED_MAKEFILE_AT} ${TOC_SHARED_MAKEFILE} || {
#        toc_die $? "Error filtering ${TOC_SHARED_MAKEFILE_AT}"
#    }
#set +x
    #echo " ${TOC_EMOTICON_OKAY}"
#    rm $makeprops
    return 0
} # end function toc_create_make


{ # function toc_find_in_path: shell function to find an app in a given path
    # usage: 
    #          toc_find_in_path appname [/path1:/path/two:path3/foo/bar]
    #
    # To keep you from having to run a subshell to assign it to a var,
    # it it sets the global var TOC_FIND_IN_PATH to the full path
    # of $1 (which may be empty, meaning it was not found).
    #
    # Sample usage:
    # toc_find_in_path ls && LS=${TOC_FIND_RESULT}
    # or (newer way):
    # toc_find_in_path ls && LS=${TOC_FIND_RESULT}
    #
    # Using ${TOC_FIND_RESULT} is more maintainable in config tests
    # because this function is used by all toc_find_XXX functions,
    # and you can rely on that variable to be set no matter which
    # of those functions you use.
    #
    # Note that TOC_FIND_RESULT is re-set on every call to this
    # function, so copy it's return value instead of referencing
    # TOC_FIND_RESULT, especially if you are calling functions
    # which may, indirectly, call this function.
    #
    # It returns 1 if ${TOC_FIND_RESULT} is empty (i.e., if it found anything),
    # else it returns 0.
    toc_find_in_path ()
    {
	toc_debug toc_find_in_path "$@"
        TOC_FIND_RESULT= # holds the quasi-return value
        local app=$1
        shift
        local path="$@"
        test -n "$path" || path="$PATH"

        # EXPERIMENTAL --with-FOO=/path/to/foo support
#        local foo
#        local bar
#        bar=$(echo $app | tr '[\-\.\\/+]' '[_____]')
#        # ^^^ there's Grief waiting to happen here :/
#        eval foo=\$configure_with_${bar}
#        test x${foo} != x -a x${foo} != x0 -a x${foo} != x1 && {
#            toc_loudly -n " Using explicit --with-$app: $foo"
#            app="${foo}"
#            path=""
#        }
        # /experiment
        
        # strange: when run as: toc_find_in_path ls $PATH
        # path is space-separated!
        # echo path=$path
        local oldifs=$IFS
        IFS=" :;"
        local found=
	test -e "$app" && found=$app || for p in $path; do
            fullp=$p/$app
            test -e $fullp || continue
            found=$fullp
	    break
        done
        #test -n "$found" || echo "App $app not found in path $path" 1>&2
        TOC_FIND_RESULT="$found"
        # echo $found
        IFS=$oldifs
        if [ -r "$found" ] ; then
            TOC_FIND_RESULT="$found"
            return 0
        else
            TOC_FIND_RESULT=
            return 1
        fi
    }
} # end funcion toc_find_in_path

{ # config manipulators

    # function add_config_{make,h}: helper functions for setting configuration args
    # These docs are only here to make these functions appear to contain
    # more arcane functionity than their sleek implementations might otherwise
    # imply.
    #
    # Only call these funcs with one key/val pair at a time, to allow for
    # proper handling of foo="something with spaces in it"
    # 
    # usage: toc_add_make VAR="VAR's value, presumably."
    toc_add_make ()
    {
        toc_debug "toc_add_make: $@"
        CONFIG_DOT_MAKE_ARRAY[${#CONFIG_DOT_MAKE_ARRAY[@]}]="$@"
    }
    toc_add_config_h ()
    {
        toc_debug "toc_add_config_h: $@"
        CONFIG_DOT_H_ARRAY[${#CONFIG_DOT_H_ARRAY[@]}]="$@"
    }
    toc_add_config ()
    {
        toc_add_make "$@"
        toc_add_config_h "$@"
    }
    toc_export ()
    { # the ultimate in laziness: toc_add_config plus shell export.
	toc_debug toc_export "$@"
	toc_add_config "$@"
	local args="$@"
	local k="${args%%=*}"
	local v="${args##*=}"
	eval "export $k='$v'"
    }

} # end config manipulator

toc_find_test ()
{ # Sets TOC_FIND_RESULT to the absolute path of the toc test named $1
  # Returns 0 if it finds a file, else 1.
    toc_find_in_path $1 . || \
        toc_find_in_path $1.sh . || \
        toc_find_in_path ${TOC_TESTSDIR}/$1 || \
        toc_find_in_path ${TOC_TESTSDIR}/$1.sh
}


{ # function toc_run test_name [args to pass to test]
    # Runs a test and returns it's return code.
    #
    # If $1 is an existing file, that file is treated
    # as the test, and sourced, otherwise a lookup
    # algorithm is used to find test_name.sh.
    #
    # Normally clients should use toc_test or toc_test_require instead
    # of this function.
    TOC_RUN_DEPTH=0
    toc_run ()
    {
        TOC_RUN_DEPTH=$((TOC_RUN_DEPTH + 1))
	local depth=${TOC_RUN_DEPTH}
        toc_debug "toc_run " "$@"
        local testname=$1 #${TOC_FIND_RESULT}
        #TOC_DIR=${TOC_TESTSDIR}
        local path=
        local desc=
        toc_find_test $testname && {
            path=${TOC_FIND_RESULT}
            desc=$(sed -ne 's|\#.*toc_run_description.*= *\(.*\)|\1|p;' ${TOC_FIND_RESULT});
            # note: ^^^ sed expr should end with 'q', but then i can only read the top-most line,
            # and i like to have a shebang as the top-most line to help out emacs :/
        } || {
            toc_die 127 toc_run cannot find script for test $testname.
        }
        shift
#        if test ${TOC_LOUD} = 1; then

        local showname="$testname"
        test "x${TOC_LOUD-0}" != "x0" -a "x$1" != "x" && showname="$showname $@"
	toc_loudly -n "[$depth]"
        i=1;
	while test $i -le $depth; do 
	    echo -n "${TOC_BOLD_}=?=${_TOC_BOLD_}"
	    i=$((i + 1))
	done
	toc_boldecho -n " [$showname] "
	test -n "$desc" && eval "echo -n $desc" 
        # ^^^ the eval allows us to use $VARS in toc_run_description.  as a
        # side-effect, however, you must escape ><| in toc_run_description!
	echo
        local err=0
        toc_source_test $testname "$@"
        err=$?
        if test ${TOC_FAIL_FAST-0} = 1 -a $err != 0; then 
	    toc_die $err "toc_run: TOC_FAIL_FAST is set, and test $testname returned a non-zero error code."
	fi
        TOC_RUN_DEPTH=$((TOC_RUN_DEPTH - 1))
        return $err
    }

    toc_repeat_echo ()
    { # Internal helper. Echoes it's arguments ($2-$N) $1 times.
        local rep=$1
        shift
        while [ $rep -ge 1 ] ; do
            echo "$@" " "
            rep=$((rep - 1))
        done
    }

    toc_test ()
    { # runs toc_run and warns if there was an error.
        # It does not exit -  it passes back the error code from the test.
        toc_debug toc_test "$@"
        local testname=$1
        local depth=$((TOC_RUN_DEPTH + 1))
        toc_run "$@" || {
            local ret=$?
            toc_loudly -n "[${depth}] "
            toc_repeat_echo 5 -n "${TOC_EMOTICON_WARNING}"
	    toc_boldecho -n "[$testname]" " "
            toc_repeat_echo 5 -n "${TOC_EMOTICON_WARNING}"
            echo
            return $ret
        }
        toc_quietly "${TOC_EMOTICON_OKAY} [$testname]"
        return 0
    }

    toc_test_require ()
    { # calls toc_run and exits if it returns !0
        toc_debug toc_test_require "$@"
        local testname=$1
        local depth=$((TOC_RUN_DEPTH + 1))
        toc_run "$@" || {
            local ret=$?
            toc_loudly -n "[${depth}] "
            toc_repeat_echo 5 -n "${TOC_EMOTICON_ERROR}"
	    toc_boldecho -n "[$testname]" " "
            toc_repeat_echo 5 -n "${TOC_EMOTICON_ERROR}"
            echo
            exit $ret
        }
        toc_quietly "${TOC_EMOTICON_OKAY} [$testname]"
        return 0
    }

    toc_source_test ()
    { # sources the given test script and returns it's error code.
      # it accepts a filename or X, where X should resolve to ${TOC_HOME}/tests/X.sh.
        local test=$1
        shift
        toc_find_test $test || toc_die $? "toc_source_test: $test not found!"
        local sh=${TOC_FIND_RESULT}
        #echo "toc_find_test: $test  sh=$sh"
        local err=0
        toc_debug "toc_source_test sourcing [$sh $@]"
        . $sh "$@"
        err=$?
        return $err
    }

    toc_find ()
    {
    # toc_find works like toc_test, but searches for the app $1
    # in path ${2-${PATH}}. It returns 1 if it finds no file, else zero.
    # It "returns" the found file in the global var ${TOC_FIND_RESULT}.
        local bin="$1"
	shift
        local path="${@-${PATH}}"
        # toc_quietly -n -e "? find $bin\t"
        toc_find_in_path $bin "$path" || {
            echo -n "${TOC_EMOTICON_WARNING} $bin"
            toc_loudly -n " not found in $path"
            echo
            return 1
        }
        local str="${TOC_EMOTICON_OKAY} $bin"
#        test "x${TOC_QUIET-0}" = "x0" && str="${TOC_EMOTICON_OKAY}"
        test "x${TOC_LOUD-0}" != "x0" && str="$str: ${TOC_FIND_RESULT}"
        toc_quietly "$str"
        return 0
    }

    toc_find_require ()
    { # Same as toc_find(), except this one exits on error
        toc_find "$@" || toc_die $? toc_find_require "$@"
        return 0
    }


} # end toc_run functions


{ 
    toc_replace_file ()
    { # toc_replace_file: helper to check if a file is changed, and replace it if it is
      # usage: toc_replace_file target tmpfile
      # tmpfile is deleted if target is the same as tmpfile, else it replaces target.
      # The fact that it echos text is a side-effect of other parts of this code,
      # so don't rely 100% on that "feature". i.e., it echos to fit in with common usage
      # of this function, not because it's a logical part of the interface :/.
      #
      # It returns 0 if it replaces $1 with $2. It returns 1 if $1 does
      # not need replacing ($1 and $2 are not different). 
      #
      #
      # It exits if it cannot mv or rm a file: that is considered a filesystem/rights-level
      # problem which needs to be addressed immediately.
      #
      #
        local tgt="$1"
        local tmp="$2"
        toc_debug toc_replace_file $tgt $tmp
        test -f "$tmp" || {
            echo "toc_replace_file: argument 2 ($2): could not find file."
            exit 1
        }
        if test -f "$tgt"; then
            if cmp -s "$tgt" "$tmp"; then
		toc_debug "toc_replace_file: $tgt is up to date. Deleting $tmp."
                #echo "toc_replace_file: $tgt fstate=up to date"
                rm -f "$tmp"
                return 1
            fi
        fi
        mv -f "$tmp" "$tgt" || {
            toc_die 2 "toc_replace_file error: toc_replace_file: Could not move [$tmp] to [$tgt]!"
        }
        return 0
    }
} # end toc_replace_file

{   
    toc_atfilter_file ()
    { 
      # uses the @-parser to filter a template file.
      # It returns !0 only if $TOC_ATSIGN_PARSER throws an error.
      # $1 = file containing properties to @-filter.
      # $2 = input template (e.g. toc_shared.make.at)
      # $3 = output file (e.g. toc_shared.make). It is only changed if
      #      needed, so it is dependencies-safe.
        local pfile="$1"
        local tmpl="$2"
        local ofile="$3"
	test -f "$pfile" || toc_die $? "toc_atfilter_file: \$1: file not found: $pfile" 
	test -f "$tmpl" || toc_die $? "toc_atfilter_file: \$2: file not found: $tmpl" 
        local tmpfile=$ofile.tmp.toc_atfilter_file
        ${TOC_ATSIGN_PARSER} -f "$pfile" < "$tmpl" > "$tmpfile" || {
            local err=$?
            echo "toc_atfilter_file(): @-parsing failed: error $err "
            echo ${TOC_EMOTICON_ERROR}
            return $err
        }
        echo -en "\t\t\t$ofile ... "

        fstate="created"
        test -f "$ofile" && fstate="updated"
        toc_replace_file "$ofile" "$tmpfile" || fstate="up to date"
        echo -en "\r${TOC_EMOTICON_OKAY} $fstate"
	echo
        return 0
    }
} # end toc_atfilter_file



toc_makerelative ()
{ # Makes a relative path from a pair of paths.
  # Usage: toc_makerelative /foo/bar/one/two /foo/bar
  # It tries to make a relative path from $1, which must
  # be a sub-path (or the same as) of $2.
  # Sets global var TOC_MAKERELATIVE to the "return value".
  # In the above example, it should = ./../..
    local startat="${1-$PWD}"
    local relto="${2-${TOP_SRCDIR}}"
    test -n "$relto" || $relto="$PWD"
    test -f "$startat" && startat="$(dirname $startat)"
    local dn=$startat/bogus
    local dn="${dn%*/*}"
    local rel="."
    while test -n "$dn" -a "$dn" != "$relto" -a "$dn" != "/"; do
        rel="$rel/.."
        dn="${dn%*/*}"
    done
    TOC_MAKERELATIVE="${rel##./}"
}



toc_internal_format_endconfigure ()
{
# internal helper to pretty up some output
    perl -ane '$| = 1;
    if( ! -f $F[0] ) { print; next; }
    $fn = $F[0];
    $st = $_;
    $st =~ s|.*\.\.\.\s+||;
    write;
format STDOUT =
  @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   @<<<<<<<<<<<<<<<<<<<<
$fn,                                                        $st
.
'
    # come on... you gotta love perl!
}

toc_endconfigure ()
{ # ends the configuration process, processing the files
  # which need to be @-parsed
        toc_boldecho "==== Creating configuration-dependent files..."
	{
            toc_test_require toc_make # | toc_internal_format_endconfigure
#	    toc_test_require toc_config_h | toc_internal_format_endconfigure
	}
        local clean="$(ls .toc.* 2> /dev/null)"
        test "x$clean" != x && rm $clean
        echo =========================================================================
        # maintenance note: the above =====... is formatted to line up with the output
        # from toc_internal_format_endconfigure
        return 0
}

toc_parseargs ()
{ # parses $@ for arguments toc can work with.
    toc_debug "toc_parseargs: Handling command-line arguments: $@"
    # mostly stolen from the qt 3.1.0 configure script :)
    local i=0
    local -a passon # clone of $@ to pass on to TOC_PARSEARGS_CALLBACK
    local configure_args
    local xtra=
    echo "$@" | grep -q \\--prefix= || xtra="--prefix=${prefix}"
#    while [ "$#" -gt 0 ]; do
    for arg in ${xtra} "$@"; do
#        local arg="$1"; shift
        passon[${i}]="$arg"
        i=$((i + 1))
        local VAR=
        local VAL=
        #echo arg=$arg
        case $arg in
            --help|-?|--?|-help)
                cat<<-EOF
		Usage:
		$0 [--prefix=/path] [--argument=value] [--arg2=val2] ...

		Standard arguments include:
EOF
# ^^^^ note usage of hard tabs in the cat data!

cat <<EOF | perl -ne '$|=1; next unless m|\w|; m/\t*(.*)\s*\|\s*(.*)/; print "    $1\n\t\t$2\n";'
	--help or -?         | Show this help.
	--help-tests         | Show a selection list of test-specific help.
	--prefix=/path       | Top-most path for installation. Default=${prefix}
	--enable-FEATURE=... | Exports config variable configure_enable_FEATURE.
	    --enable-FEATURE     | Equivalent to --enable-FEATURE=1.
	    --disable-FEATURE    | Equivalent to --enable-FEATURE=0.
	--with-FEATURE=...   | Sets the config variable configure_with_FEATURE.
	    --with-FEATURE       | Equivalent to --with-FEATURE=1.
	    --without-FEATURE    | Equivalent to --with-FEATURE=0.
	--XXX=YYY            | Exports config variable XXX=YYY.
	--XXX                | Equivalent to --XXX=1.
	--toc-quiet,--quiet  | Makes configure run more quietly.
	--toc-loud, --loud | configure will output some otherwise superfluous information.
	--verbose           | Enables --loud and disables --quiet.
	--toc-fail-fast      | Causes configure to abort if toc_test returns non-zero.
	--toc-debug          | Enables debugging output in the configure framework.
EOF
# ^^^^ note usage of hard tabs in the cat data!
cat <<EOF

    Many configure tests use --XXX and --with/enable-XXX. Try using
--help-testname to get test-specific help. e.g. --help-gnu_cpp_tools
or --help-zlib. (This does not work for all tests: try --help-tests
if it does not work for a given test.)

EOF


                local help=${TOP_SRCDIR}/toc.${PACKAGE_NAME}.help
                test -f $help && {
                    echo -e "\nHelp options specific to ${PACKAGE_NAME}:\n"
                    local foo=$(head -1 $help)
                    local sh=${foo#\#\!}
                    local hash=${foo%%\!*}
                    test "x#" = "x$hash" -a "x$hash" != "x$sh" -a "x$sh" != "x" || {
                        # lookie like no script of some sort.
                        sh=cat
                    }
                    $sh $help
                    echo "" # just for looks.
                }
                local help=toc.${PACKAGE_NAME}.help.sh
                test -f $help && {
                    echo ""
                    echo "Help options specific to this package:"
                    cat $help
                    echo "" # just for looks.
                }

                exit 0
                ;; # finally end --help
            --prefix=*)
                # reserved.
                VAR=$(echo $arg | sed "s,^--\(.*\)=.*,\1,;")
                VAL=$(echo $arg | sed "s,^--.*=\(.*\),\1,")
                ;;
	    --loud|--toc-loud)
	       VAR=TOC_LOUD
	       VAL=1
	       ;;
            --verbose)
                TOC_QUIET=0
                TOC_LOUD=1
                export TOC_QUIET TOC_LOUD
                ;;
	    --quiet)
	       VAR=TOC_QUIET
	       VAL=1
	       ;;
            --TOC-[-_]*|--toc[-_]*)
                # reserved.
                VAR=$(echo $arg | sed "s,^--\(.*\)=.*,\1,;s,-,_,g;s/^__//;" | tr '[a-z\-]' '[A-Z_]')
                VAL=$(echo $arg | sed "s,^--.*=\(.*\),\1,;/^--/d;")
                test -n "$VAL" || VAL=1
                ;;
           --enable-*=*|--with-*=*)
                # sets configure_with_XXX or configure_enable_XXX
                VAR=$(echo "$arg" | sed "s,^--\(.*\)=.*,\1,;s,-,_,g")
                VAL=$(echo "$arg" | sed "s,^--.*=\(.*\),\1,")
                VAR=configure_${VAR}
                ;;
           --enable-*|--with-*)
                # sets configure_with_XXX=1 or configure_enable_XXX=1
                VAR=configure_$(echo $arg | sed "s,^--\(.*\),\1,;s,-,_,g")
                VAL=1
                ;;
            --disable-*)
                # sets configure_enable_XXX=0
                VAR=configure_$(echo $arg | sed "s,^--disable\(.*\),enable\1,;s,-,_,g")
                #  This needs to be 0, not "", because otherwise you wind up
                #  with "#define HAVE_FOO" when it's been unset, which is not
                #  desirable.  This way, at least you get "#define HAVE_FOO 0".
                VAL=0
                ;;
            --without-*)
                # sets configure_with_XXX=0
                VAR=configure_$(echo $arg | sed "s,^--without\(.*\),with\1,;s,-,_,g")
                #  This needs to be 0, not "", because otherwise you wind up
                #  with "#define HAVE_FOO" when it's been unset, which is not
                #  desirable.  This way, at least you get "#define HAVE_FOO 0".
                VAL=0
                export $VAR="$VAL"
                ;;
            --*=*) # SOOOOOOO arguable
                # sets XXX=YYY
                VAR=$(echo $arg | sed "s,^--\(.*\)=.*,\1,;s,-,_,g")
                VAL=$(echo $arg | sed "s,^--.*=\(.*\),\1,")
                ;;
            --*) # this is soooooo arguable
                # sets XXX=1
                VAR=$(echo $arg | sed "s,^--\(.*\),\1,;s,-,_,g;")
                VAL=1
                ;;
            *)
                toc_boldecho "WARNING: unrecognized option: $arg"
            ;;
        esac
        toc_debug toc_parseargs: $arg "VAR=$VAR VAL=$VAL"
        test -n "$VAR" -a -n "$VAL" && {
                toc_debug "toc_parseargs: exporting [$VAR]=$VAL"
                export $VAR="$VAL"
                toc_export "$VAR=$VAL"
#		foo=$(echo $VAL | sed -e "s,',\\\\',g")
		configure_args="$configure_args --$VAR=\"$VAL\""
        }
    done
    toc_add_config CONFIGURE_ARGUMENTS="$configure_args" # <---- !!!!experimental!!!!
    test "x${TOC_PARSEARGS_CALLBACK}" != "x" && {
        ${TOC_PARSEARGS_CALLBACK} "${passon[@]}"
        return $?
    }
    return 0
} # end parseargs


# We do some initial sanity checking here,
# but any tree-specific checks must be done from configure.

toc_parseargs "$@"
# WTF??? toc_get_make prefix || toc_add_config prefix=${prefix}

toc_loudly 'Loud mode enabled.'

toc_source_test toc_tests_help || exit 0

echo "Configuring build environment."

toc_quietly <<-EOF
${TOC_BOLD_}Emoticon legend:${_TOC_BOLD}
  ${TOC_EMOTICON_OKAY}  Good News.
  ${TOC_EMOTICON_ERROR}  Bad News.
  ${TOC_EMOTICON_WARNING}  A warning, possibly even Bad News, but we do not yet
       know if it is fatal: this is build tree dependent.

	Note that not all config test failures are fatal: it will stop if
	it fails a test which it /needs/ to pass.
EOF

# . ${TOC_HOME}/tests/toc_core_tests.sh || {
toc_test_require toc_core_tests || {
    err=$?
    echo "Core toc sanity test failed (err=$err). This tree is not toc-capable. ${TOC_EMOTICON_ERROR}"
    exit $err
}

toc_grep_tests()
{ # a client-helper func to grep a list of toc test names from an input file.
  # It echos the list of tests
        test -z "$1" && toc_die 1 "toc_grep_tests: error: \$1 must be a filename."
        sed -n -e 's/^ *//;/^toc_test/p;' $1 | cut -d' ' -f 2
}

# toc_add_make TOC_CLEAN_FILES=""
return 0
