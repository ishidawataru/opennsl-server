#!/do/not/bash
# toc_run_description = creating included makefiles
# Do not run this test manually: it is run by the toc core.


TOC_MAKE=toc.make
TOC_QMAKE=toc.qmake

toc_add_make TOC_MAKE=${TOC_MAKE}
toc_add_make TOC_QMAKE=${TOC_QMAKE}

# set -x
toc_make_toc_make ()
{
    # use a function because bash doesn't allow local vars outside of functions.
    local usage="usage: arg1==target makefile basename. arg2=input template"
    local themake=${1?$usage}
    local themakein=${2?$usage}
    echo "Creating $themake ... "
    local makeprops=${TOP_SRCDIR}/.toc.make.tmp
    toc_dump_make_properties > $makeprops
    local thedir
    local tocmake
    local relpath
    local shortform
    local tocmakeprops
    for d in $(find ${TOP_SRCDIR} -name Makefile -o -name GNUMakefile -o -name Makefile.toc -o -name '*.qmake' \
        | xargs grep -E -l "include.+${themake}" | sort -u); do
        #echo "d=$d" >&2
        thedir=$(dirname $d)
	tocmake=${thedir}/$themake
        toc_makerelative $thedir
        relpath=${TOC_MAKERELATIVE}
	tocmake=${tocmake##$TOP_SRCDIR/}  # make it short, for asthetic reasons :/
        shortform=${thedir##${PWD}/}
        test "$shortform" = "$PWD" && shortform= # top-most dir
#        echo "tocmake=$tocmake relpath=$relpath shortform=$shortform"

        tocmakeprops=${cmake}.props
        cp $makeprops $tocmakeprops
        cat <<EOF >> $tocmakeprops
TOP_SRCDIR=${relpath##./}
TOC_RELATIVE_DIR=${shortform##./}
EOF
        toc_atfilter_file $tocmakeprops $themakein $tocmake \
            || toc_die $? "Error creating $themake!"
        rm $tocmakeprops
        touch $tocmake # required for proper auto-reconfigure :/
    done
}

toc_make_toc_make ${TOC_MAKE} ${TOC_HOME}/make/toc.make.at
err=$?
test $err != 0 && {
    echo "Error creating ${TOC_MAKE} makefiles!"
    return $err
}

test -f ${TOP_SRCDIR}/toc.${PACKAGE_NAME}.qmake.at && {
    toc_make_toc_make ${TOC_QMAKE} ${TOC_HOME}/make/toc.qmake.at
    err=$?
    test $err != 0 && {
        echo "Error creating ${TOC_QMAKE} makefiles!"
        return $err
    }
}

return $err





