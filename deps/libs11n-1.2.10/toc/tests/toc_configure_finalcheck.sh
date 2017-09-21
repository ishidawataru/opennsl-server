# does some closing-time checks on the configure environment.

ret=0
for ifile in \
    ${TOP_SRCDIR}/toc_shared.make ${TOP_SRCDIR}/include/config.h \
    ; do \
	grep -iqe "@[A-Z0-9_][A-Z0-9_]*@" $ifile > /dev/null && {
	    boldecho -n "WARNING: "
	    echo "$ifile contains un-parsed @tokens@. This may or may not be an error. ${TOC_EMOTICON_WARNING}"
	    ret=1
	    # note that this check is broken because it is generic:
	    # we should not include @tokens@ which are commented-out in makefiles,
	    # but the makefile comment delimiter is the first char of the #define
	    # directive for config.h.at.
	    # one day i'll code separate tests for each file.
	}

    done

return $ret