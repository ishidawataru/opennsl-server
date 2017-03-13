# toc_run_description = looking for GNU flex
# toc_begin_help =
# Looks for flex in $1 (defaults to $PATH) and FlexLexer.h
# in some common paths. It calls toc_export for the following
# variables:
#
#	FLEX=/path/to/flex (maybe empty)
#	FLEX_BIN= same thing, but preferred for style's sake
#	FLEXLEXER_H=/path/to/FlexLexer.h
#
# It honors the following configure arguments:
#
#  --with-flex=/path/to/flex
#  --with-flexlexer=/path/to/FlexLexer.h
#  --without-flex sets FLEX to an empty string.
#
# It returns zero if flex is found, else non-zero.
# A missing FlexLexer.h is not considered an error, since it
# is only useful for C++
#
# = toc_end_help

if [ x0 = x${configure_with_flex-1} ]; then
    echo "Explicitely disabled via --without-flex."
    toc_export FLEX=
    toc_export FLEX_BIN=
    toc_export FLEXLEXER_H=
    return 0
fi

toc_find ${configure_with_flex-flex} flex ${1-${PATH}}
err=$?
toc_export FLEX=${TOC_FIND_RESULT}
toc_export FLEX_BIN=${TOC_FIND_RESULT}

toc_find ${configure_with_flexlexer-FlexLexer.h} /usr/include:/usr/local/include:$PWD/include
toc_export FLEXLEXER_H=${TOC_FIND_RESULT}

return $err

