#!/do/not/bash
# toc_run_description = Looking for required build components...
# toc_begin_help =
# To be sourced from toc_core.sh. This is the core sanity checker for
# toc. Any test which MUST pass for a tree to be considered
# toc-capable may be placed in here.
# = toc_end_help

toc_run running_under_cygwin
# toc_quietly "Looking for required build components:"
for x in \
    bash=SHELL \
    cat=CAT \
    cut=CUT \
    ls=LS \
    perl=PERL \
    sed=SED \
    xargs=XARGS \
    ; do
    f=${x%%=*}
    v=${x##*=}
    toc_find $f || {
        echo "configure couldn't find required app: $f"
        return 1
    }
    test "x$v" = "xXtocX" && continue
    toc_add_make $v=${TOC_FIND_RESULT}
done

toc_find less || toc_find more || {
    echo "Found neither 'more' nor 'less'!!!"
    return 1
}

toc_find_require install-sh ${TOC_HOME}/bin
# toc_add_make INSTALLER_BIN="\$(top_srcdir)/${TOC_FIND_RESULT##${PWD}/}"
toc_add_make INSTALLER_BIN="${TOC_FIND_RESULT}"

toc_find_require makedist ${TOC_HOME}/bin
toc_add_make TOC_MAKEDIST_BIN="${TOC_FIND_RESULT}"
# it's very arguable to make makedist a required component :/

for x in \
    awk \
    gnu_make \
    gnu_find \
    gnu_tar \
    ; do
    toc_test $x || {
        echo "${TOC_EMOTICON_ERROR} $x test failed."
        return 1
    }
done

#toc_test gnu_install || {
#    boldecho "Didn't find GNU install. You won't be able to do a 'make install'."
#}

toc_quietly "Looking for optional build components:"
for x in \
    gzip=GZIP \
    bzip2=BZIP \
    zip=ZIP \
    ; do
    f=${x%%=*}
    v=${x##*=}
    foo=configure_with_${f}
    toc_find $f
    toc_add_make $v=${TOC_FIND_RESULT}
done

return 0
