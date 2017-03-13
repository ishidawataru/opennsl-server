#!/do/not/bash
# toc_run_description = creating config.h
# toc_begin_help =
#   Creates a config.h from config.h.at.
#   Accepts optional $1 naming a target config.h-like
#   file. The file $1.at must exist for this to work.
# = toc_end_help

toc_config_h ()
{   # $1 = target file. $1.at must exist.
    # todo: add support for multiple input files.
    # i use a function so i can use local vars
    local propsfile=.config.h.properties
    local configh=$1
    configh=${configh##${TOP_SRCDIR}/}
    local tmpl=$configh.at
    test -f $tmpl || {
        echo "Note: no $tmpl found, so $configh will not be created."
        return 0
    }
    i=0
    test -f "$propsfile" && rm "$propsfile"
    toc_dump_config_h_properties > $propsfile
    toc_atfilter_file $propsfile $tmpl $configh
    rm $propsfile
    return 0
}

files="$@"
test -z "$files" && files=${TOC_INCLUDESDIR}/config.h
toc_config_h "$files"
return $?

