#!/do/not/bash
# toc_run_description = checking for P::Classes libs and headers...
# toc_begin_help =
# Checks for the existance of the P::Classes libs and headers.
#
# Exports the following variables:
#
#   HAVE_PCLASSES = 0 or 1
#
#   PCLASSES_LDADD = -Lpclasses_prefix -l... (each pXXX lib)
#
# = toc_end_help


paths="${configure_with_pclasses-${prefix}}:/usr:/usr/local:$HOME"
toc_debug "pclasses search path=[$paths]"

ret=0
pprefix=
for H in pany.h psqldriver.h piomanager.h; do
    toc_find include/pclasses/$H "$paths" || {
        ret=1
        break
    }
    test x != "x${pprefix}" && continue
    pprefix="${TOC_FIND_RESULT%%/include/pclasses/*}"
done

pldadd=""
for L in pcore pcrypto pio pnet psql; do
    toc_find lib/lib${L}.so "$paths" || {
        ret=1
        break
    }
    pldadd="${pldadd} -l$L"
done

toc_export PCLASSES_PREFIX="${pprefix}"

test 0 = $ret && {
    toc_export PCLASSES_LDADD="-L${pprefix}/lib -L${pprefix}/lib/pclasses ${pldadd}"
    toc_export HAVE_PCLASSES=1
} || {
    toc_export HAVE_PCLASSES=0
}

unset pldadd paths pprefix
return $ret

