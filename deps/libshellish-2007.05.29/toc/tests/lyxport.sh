#!/do/not/bash
# toc_run_description = looking for lyxport
# toc_begin_help = 
# Looks for the lyxport tool (http://www-hep.colorado.edu/~fperez/lyxport).
# Use --without-lyxport to disable it. It exports these variables:
#

# - LYXPORT_BIN=/path/to/lyxport
# - LYXPORT_MAKE=/path/to/lyxport.make (part of toc)

# Additionally, it *might* export the following:
#
#
# - PS2PDF_BIN
# - LYX_BIN
# - LATEX_BIN
# - LATEX2HTML_BIN
# - DVIPS_BIN
# - DVI2TTY_BIN
#
# Usage: put something like the following in your makefile template:
#
#  configure_with_lyxport = @configure_with_lyxport@
#  ifeq (1,$(configure_with_lyxport))
#  LYXPORT_L2HARGS = -nonavigation -show_section_numbers -split 0 -noimages
#  LYXPORT_ARGS = -c -tt
#  LYXPORT_ARGS += --html --opts_l2h '$(LYXPORT_L2HARGS)'
#  LYXPORT_ARGS += --ps --pdf
#  endif
#
# = toc_end_help

toc_export LYXPORT_BIN=
toc_export LYXPORT_MAKE=
toc_add_make configure_with_lyxport=0

test "x${configure_with_lyxport-1}" = "x0" && {
    echo "lyxport has been explicitely disabled."
    return 1
}
configure_with_lyxport=${configure_with_lyxport-1}

lyxport=lyxport
test x1 != x${configure_with_lyxport} && lyxport=${configure_with_lyxport}

path=${PATH}:${TOC_HOME}/bin
toc_find $lyxport $path || {
    echo "lyxport not found. You can possibly find lyxport at http://www-hep.colorado.edu/~fperez/lyxport"
    return 1
}
lyxport=${TOC_FIND_RESULT}
lyxport_makefile=${TOC_HOME}/make/lyxport.make
toc_find $lyxport_makefile || {
    echo "lyxport requires $lyxport_makefile"
    return 1
}
lyxport_make=${TOC_FIND_RESULT}


auxtools="lyx latex latex2html dvips ps2pdf dvi2tty"
foundtools=0
for i in $auxtools; do
    toc_find $i && foundtools=1
    test "$i" = "lyx" -a -z "${TOC_FIND_RESULT}" && {
        echo "Required app $i not found in PATH."
        return 1
    }
    fn=${TOC_FIND_RESULT}
    big=$(echo $i | tr [:lower:] [:upper:])_BIN
    toc_export ${big}=$fn
done

test "x$foundtools" = "x0" && {
    echo "None of the auxillary tools for lyxport found: $auxtools"
    return 1
}

toc_export LYXPORT_BIN=${lyxport}
toc_export LYXPORT_MAKE=${lyxport_make}
toc_export configure_with_lyxport=${configure_with_lyxport}

return 0
