# toc_run_description = search for a genuine GNU make, the Make of makes.

toc_find gmake || toc_find make || return
toc_export MAKE=${TOC_FIND_RESULT}
"$MAKE" --version | grep -qi GNU > /dev/null || {
        echo "Your make appears to be non-GNU."
        return 1
}

