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
TOC_HOME does not point to a toc installation. Do one of the following:

1) set TOC_HOME to the path containing toc/bin and toc/sbin, e.g.,
   export TOC_HOME=/path/to/toc

2) copy or symlink the 'toc' tree into this directory. If the original
   is called toc-VERSION, then name the copy/symlink 'toc'.

EOF
    exit 1
}

echo "Using TOC_HOME: ${TOC_HOME}"
