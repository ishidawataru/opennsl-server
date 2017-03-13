#!/do/not/bash
# toc_run_description = looking for CVS client tools
# toc_begin_help = 
# Looks for the cvs client tools. Exports:
#
# - CVS_BIN=/path/to/cvs
#
# It honors the configure argument --with-cvs=/path/to/cvs
# = toc_end_help


cvs=${configure_with_cvs-cvs}

toc_find $cvs
toc_export CVS_BIN=${TOC_FIND_RESULT}

test x = "x${CVS_BIN}" && return 1

return 0
