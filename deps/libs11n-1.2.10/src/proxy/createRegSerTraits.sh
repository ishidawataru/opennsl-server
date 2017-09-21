#!/bin/bash
# Docs are below...

########################################################################
function show_help()
{
    cat <<EOF
$0:

Creates s11n Serializable class registration code for s11n using a
traits registration supermacro.

Parameters: (* = required)

*   -c SerializableClass

    -b BaseInterfaceClass               [${o_base}]
       The base Serializable type. i.e., that used in calls
       to de/serialize<Base>() and friends. Default is
       the -c class.

    -a
       The SerializableClass is Abstract (or otherwise
       cannot or shoult not be instantiated by the
       classloader). Default is false.

    -n SerializableClassName           [${o_clname}]
       Default is that from -c.

    -r <inc/registration_header.h>     [${o_reg_h}]

    -p SerializableProxyClass          [${o_proxy}]
       Sets the s11n proxy class. SerializableClass
       must conform to the s11n interface supported
       by this proxy. The default proxy requires
       that SerializableClass implement two
       operator()(NodeType) overloads, as described
       in the libs11n manual.

    -list
       SerializableClass is compatible with the
       std::list proxy, and should use it.
       The -p option will trump the default list
       proxy, making this a no-op if -p is used.

    -list-ios
       Like -list, but use a proxy optimized for use
       with a container of an i/ostreamable type.

    -map
       Like -list, but applies to map-compatible types.

    -map-ios
       Like -list-ios, but for maps. Both key and value types
       must be i/ostreamable.

    -ios
       Like -list, but installs a proxy which works using
       the i/ostream operators of SerializableClass.

    **************************************************
    Options intended for use by the s11n maintainer:
    **************************************************

    -macro-prefix XXX                  [${o_macro_prefix}]
       Prepends XXX to the start of the reg macros.

    -ns namespace_of_s11n              [${o_sns}]
        An obscure option to help me port code between
	pclasses.com::s11n and s11n.net::s11n.
	If used, this option needs to be as early
	in the args list as possible (BUG: must come
	before any options which rely on the namespace).
	e.g.: -ns P::s11n

EOF
} # show_help()

o_sns="::s11n" # must unfortunately have default val up here.
# proxy vars: yes, the single-quotes are intentional
i_proxy_list='${o_sns}::list::list_serializable_proxy'
i_proxy_list_s='${o_sns}::list::streamable_list_serializable_proxy'
i_proxy_map='${o_sns}::map::map_serializable_proxy'
i_proxy_map_s='${o_sns}::map::streamable_map_serializable_proxy'
i_proxy_s='${o_sns}::streamable_type_serialization_proxy'
o_macro_prefix=  # prefix for registration macros. Use 'P' for P::s11n, empty for ::s11n

############################## args parsing...
while test x != "x$1"; do
    a=$1
    shift
    case $a in
	-a)
	    o_abstract=1
	    continue
	    ;;
	-b)
	    o_base="$1"
	    shift
	    continue
	    ;;
	-c)
	    o_class="$1"
	    shift
	    continue
	    ;;
	-ns)
	    o_sns="$1"
	    shift
	    ;;
	-n)
	    o_clname="$1"
	    shift
	    continue
	    ;;
	-r)
	    o_reg_h="$1"
	    shift
	    continue
	    ;;
	-p)
	    o_proxy="$1"
	    shift
	    continue
	    ;;
	-ios)
	   : ${o_proxy="$(eval echo ${i_proxy_s})"}
	    continue
	    ;;
	-list)
	    : ${o_proxy="$(eval echo ${i_proxy_list})"}
	    continue
	    ;;
	-list-ios)
	    : ${o_proxy="$(eval echo ${i_proxy_list_s})"}
	    continue
	    ;;
	-map)
	    : ${o_proxy="$(eval echo ${i_proxy_map})"}
	    continue
	    ;;
	-map-ios)
	    : ${o_proxy="$(eval echo ${i_proxy_map_s})"}
	    continue
	    ;;
	-macro-prefix)
	    o_macro_prefix="$1"
	    shift;
	    continue;
	    ;;
	-?|--help|-h)
	    o_show_help=1
	    # We collect all the args before showing help
	    # so we can show them in context in the help
	    # text.
	    continue
	    ;;
	*)
	    echo "Unknown argument: $a"
	    exit 2
    esac
done

############################## arg defaults...
: ${o_abstract=0}
: ${o_proxy="${o_sns}::default_serialize_functor"}
: ${o_clname="${o_class}"}
: ${o_base=${o_class}}
: ${o_reg_h="<s11n.net/s11n/reg_s11n_traits.hpp>"}
: ${o_show_help=0}



test x1 = "x${o_show_help}" && {
    show_help
    exit 0
}

test x = "x${o_class}" && {
    show_help
    exit 1
} # end usage text


do_proxy_reg=1

DEFP=${o_macro_prefix}
test ${o_sns} = "s11n" && DEFP="" # the s11n.net tree

test x1 = "x${do_proxy_reg}" && {

    # Note the use of HARD TABS in the <<-EOF!!!
    cat <<-EOF
	#define ${DEFP}S11N_TYPE ${o_class}
	#define ${DEFP}S11N_TYPE_NAME "${o_clname}"
	#define ${DEFP}S11N_SERIALIZE_FUNCTOR ${o_proxy}
	EOF

    test "${o_class}" != "${o_base}" && \
	echo "#define PS11N_TYPE_INTERFACE ${o_base}"

    test x1 = "x${o_abstract}" && \
	echo "#define PS11N_TYPE_NAME \"${o_clname}\""

    echo "#include ${o_reg_h}"
}
########################################################################
