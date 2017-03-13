# toc_run_description = looking for gawk/awk/nawk

err=0
toc_find gawk || toc_find awk || toc_find nawk || err=1
toc_add_make AWK=$TOC_FIND_RESULT
return $err
