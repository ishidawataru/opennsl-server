
for foo in \
    gcc cc \
    ; do
    toc_find $foo || continue
    CC=${TOC_RUN_FIND_FAILOK}
    echo C compiler: ${TOC_RUN_FIND_FAILOK}
    break
done

for foo in \
    g++ gcc c++ \
    ; do
    toc_find $foo || continue
    CXX=${TOC_RUN_FIND_FAILOK}
    echo C++ compiler: ${TOC_RUN_FIND_FAILOK}
    break
done

export CC CXX
toc_add_make CC=${CC}
toc_add_make CXX=${CXX}

return 0
