#!/bin/bash

test x = "x${TOC_HOME}" && {
    for d in $PWD/toc $PWD/../toc $HOME/toc /usr/share/toc /usr/local/share/toc; do
        test -e $d/sbin/toconfigure -a -e $d/bin/mkdep.c && {
            cd $d > /dev/null
            export TOC_HOME=$PWD
            cd - > /dev/null
            echo "Found TOC_HOME: ${TOC_HOME}"
            break
        }
    done
}

test x = "x${TOC_HOME}" && {
    cat <<EOF
The build tools were not found :(.

You can download toc from http://s11n.net/download/

After obtaining toc, do ONE of the following:

1) Set the TOC_HOME environment variable to the path containing toc/bin
   and toc/sbin. For example, in the bash shell one would use:
   export TOC_HOME=/path/to/toc

2) Copy or symlink the 'toc' tree into the s11n source directory OR one
   directory above the s11n tree. If the original is called toc-VERSION,
   then name the copy/symlink 'toc'.

EOF
    exit 1
}

echo "Using TOC_HOME: ${TOC_HOME}"
