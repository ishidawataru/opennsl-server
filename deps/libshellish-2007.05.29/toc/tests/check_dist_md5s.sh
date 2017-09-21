# toc_run_description = checking MD5 sums of your distribution

distname=${PWD##*/}
fn=${1-md5.sums.${distname}}

test ! -f $fn && {
    echo "File $fn not found. Skipping md5 check."
    return 0;
}

toc_find_in_path md5sum || {
    echo "md5sum not found in your PATH. Skipping md5 check! ${TOC_EMOTICON_WARNING}"
    return 0
}
md5=${TOC_FIND_RESULT}
tmp=.toc.checkmd5.foo
ret=0
$md5 -c $fn > $tmp || {
    grep FAILED $tmp
    echo "md5 check failed! Your distribution does not seem to be an unmodified copy of ${PACKAGE_NAME}!"
    ret=1
}
rm $tmp
return $ret
