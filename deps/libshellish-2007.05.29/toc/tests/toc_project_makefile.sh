# Creates toc.${PACKAGE_NAME}.make from toc.${PACKAGE_NAME}.make.at
# This file is sought by toc.make (but is always optional)

mf=${1-toc.${PACKAGE_NAME}.make}
tmpl=$mf.at
test -f $tmpl || {
        toc_add_make toc_include_project_makefile=0
        echo "No project-specific makefile found at [$mf]."
        echo "This is /probably/ not a problem."
        return 1
}

toc_atfilter_as_makefile $tmpl $mf || {
    err=$?
    echo "Error filtering $tmpl!"
    return $err
}

# toc_add_make toc_include_project_makefile="\$(top_srcdir)/${mf}"



