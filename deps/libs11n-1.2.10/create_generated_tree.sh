#!/bin/bash
# Creates a copy of the source tree with all generated/symlinked files
# in place, suitable for building under more arbitrary build environments.
# Or tries to, anyway.
# Requires a completely-built copy of the s11n tree.

target_dir=${PWD}/nobuildfiles
test -d ${target_dir} || mkdir -p ${target_dir}

### Input source files
in_sources="
src/s11n/s11n.cpp
src/s11n/s11n_node.cpp
src/s11n/exception.cpp
src/s11n/export.hpp
src/io/data_node_io.cpp
src/io/strtool.cpp
src/io/compact/compact.flex.cpp
src/io/compact/compact_serializer.cpp
src/io/expat/expat_serializer.cpp
src/io/funtxt/funtxt.flex.cpp
src/io/funtxt/funtxt_serializer.cpp
src/io/funxml/funxml.flex.cpp
src/io/funxml/funxml_serializer.cpp
src/io/parens/parens.flex.cpp
src/io/parens/parens_serializer.cpp
src/io/simplexml/simplexml.flex.cpp
src/io/simplexml/simplexml_serializer.cpp
src/io/wesnoth/wesnoth.flex.cpp
src/io/wesnoth/wesnoth_serializer.cpp
src/lite/s11nlite.cpp
src/plugin/path_finder.cpp
src/plugin/plugin.cpp
src/plugin/plugin.win32.cpp
src/plugin/plugin.dl.cpp
src/plugin/plugin.noop.cpp
src/in.sxml
src/in.xml
src/in.wes
src/client/s11nconvert/main.cpp
src/client/s11nconvert/argv_parser.hpp
src/client/s11nconvert/argv_parser.cpp
"


echo "Copying headers to ${target_dir}/..."
test -e include/s11n.net/s11n/s11nlite.hpp || {
    echo "It seems your source tree has not yet been built. Build it before running this."
    exit 1
}
########################################################################
# Copy the headers, dereferencing the symlinks to create copies...
tar cf - -h include/s11n.net | (
    cd ${target_dir} || exit
    tar xpf - || exit
)


echo "Copying sources to ${target_dir}/..."
srctgt=${target_dir}/src
test -d ${srctgt} || mkdir -p ${srctgt}
for s in ${in_sources}; do
     cp -p $s ${srctgt}/${s##*/} || exit
done

echo "Kludgy changes of some files..."
s11nincdir=${target_dir}/include/s11n.net/s11n
pluginconf=${s11nincdir}/plugin/plugin_config.hpp
echo "Disabling libdl/libltdl in $pluginconf..."
perl -i -pe 's|(s11n_CONFIG_HAVE_LIBLTDL) +.*|$1 0|;s|(s11n_CONFIG_HAVE_LIBDL) +.*|$1 0|' $pluginconf

sconfig=${s11nincdir}/s11n_config.hpp
echo "Disabling zfstream and expat support in $sconfig..."
perl -i -pe 's|(s11n_CONFIG_HAVE_ZFSTREAM) +.*|$1 0|;s|(s11n_CONFIG_HAVE_LIBEXPAT) +.*|$1 0|' $sconfig

echo "Installing READMEs and rudimentary Makefile..."
cp -p README.WIN32 README.src INSTALL.nobuildfiles LICENSE.libs11n ${target_dir}
mf=${PWD}/src/Makefile.for.generated.src.tree
cp -p $mf ${srctgt}/Makefile

echo "Finished. Output is in:"
echo ${target_dir}
echo "Now please see the README(s) in the source tree."


