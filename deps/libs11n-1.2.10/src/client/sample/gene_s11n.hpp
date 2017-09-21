#ifndef gene_S11N_HPP_INCLUDED
#define gene_S11N_HPP_INCLUDED 1
////////////////////////////////////////////////////////////////////////
// This header implements libs11n (1.1+) proxies for de/serializing
// data types from the gene namespace.
// User is assumed to have included gene.hpp by this point.
//
// License: public domain
// Author: stephan@s11n.net, August 2005
////////////////////////////////////////////////////////////////////////

/**
   gene_USE_HEAVYWEIGHT_S11N determines whether we can use more
   compact s11n algos for some maps. If XML compatibility is a
   concern, this should be set to one so we use different algos which
   work for all s11nable types, but generate much larger output than
   we can generate when we know that the file format can handle
   numeric node keys.

   Set to 0 for XML-compatible, 1 for more compact, but non-XML-compliant, output.

   ACHTUNG: client code may install its own proxy for Genome<X>::gene_map, and there
   is no guaranty that its algo is XML compliant.

   Reminder: enabling this will increase compile times notably (by 4 seconds
   in one of my tests), mainly due to the additional proxies needed for map keys
   and values.
*/
#define gene_USE_HEAVYWEIGHT_S11N 0 

#if gene_USE_HEAVYWEIGHT_S11N
#  include <s11n.net/s11n/proxy/std/map.hpp> // proxy std::map<>
#else
#  include <s11n.net/s11n/proxy/mapish.hpp> // map-related algos, but no registration
#endif

namespace gene {


	/** s11n proxy for Gene<IoStreamableType>. */
	struct Gene_s11n
	{
		// serialize operator:
		template <typename NodeType, typename GeneT>
		bool operator()( NodeType & dest, const GeneT & src ) const
		{
			typedef s11n::node_traits<NodeType> NTR;
			NTR::set( dest, "v", src.value() );
			return true;
		}

		// deserialize operator:
		template <typename NodeType, typename GeneT>
		bool operator()( const NodeType & src, GeneT & dest ) const
		{
			typedef s11n::node_traits<NodeType> NTR;
			dest.value( NTR::get( src, "v", dest.value() ) );
			return true;
		}
	};

	/**
	   s11n proxy for Genome< Gene<IoStreamableType>, IoStreamableType>.

	   If gene_USE_HEAVYWEIGHT_S11N is true then this class
	   requires that the contained types in
	   Genome<Gene<SerializableType1>,SerializableType2> all be
	   Serializable, but i/ostreamable types can be proxied to
	   Serializables with little effort.
	 */
	struct Genome_s11n
	{
		// serialize operator:
		template <typename NodeType, typename GenomeT>
		bool operator()( NodeType & dest, const GenomeT & src ) const
		{
			typedef s11n::node_traits<NodeType> NTR;
#if gene_USE_HEAVYWEIGHT_S11N
			return s11n::serialize_subnode( dest, "genes", src.genes() );
#else
			return s11n::map::serialize_streamable_map( dest, "genes", src.genes() );
#endif
			// i prefer the streamable variant, but if genomes use numeric keys,
			// XML-based serializers aren't guaranteed to be able to read the data.
			// Thus we may want to use a more data-inflated, but ultimately more
			// data-portable, approach. That said, s11n's funxml and simplxml formats
			// accepts double/int keys, in blatant violation of XML standards,
			// as do all other current s11n io formats except for the expat serializer.
		}

		// deserialize operator:
		template <typename NodeType, typename GenomeT>
		bool operator()( const NodeType & src, GenomeT & dest ) const
		{
#if gene_USE_HEAVYWEIGHT_S11N
			return s11n::deserialize_subnode( src, "genes", dest.genes() );
#else
			return s11n::map::deserialize_streamable_map( src, "genes", dest.genes() );
#endif
		}

	};


} // namespace gene

////////////////////////////////////////////////////////////////////////
// Register Gene_s11n as the proxy for Gene<ValueType>: ValueType
// *must* be i/ostreamable.  If (gene_USE_HEAVYWEIGHT_S11N) then
// ValueType *must* be a Serializable. For PODs this can be
// accomplished by including an s11n proxy for the type. e.g.:
// #include <s11n.net/s11n/proxy/pod/double.hpp>
// #include <s11n.net/s11n/proxy/pod/string.hpp>
// #include <s11n.net/s11n/proxy/pod/int.hpp>
#define S11N_TEMPLATE_TYPE gene::Gene
#define S11N_TEMPLATE_TYPE_NAME "gene::Gene"
#define S11N_TEMPLATE_TYPE_PROXY gene::Gene_s11n
#include <s11n.net/s11n/proxy/reg_s11n_traits_template1.hpp>
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Register Genome_s11n as the proxy for Genome<GeneType,KeyType>:
// GeneType is expected to be gene::Gene<ValueType>.
// The proper includes for POD KeyType are the same as for Gene<ValueType>
// (see above).
// If (!gene_USE_HEAVYWEIGHT_S11N) then KeyType *must* be i/ostreamable
// but need not otherwise be a Serializable, otherwise KeyType must
// be a Serializable (e.g., a POD proxies via an s11n proxy).
#define S11N_TEMPLATE_TYPE gene::Genome
#define S11N_TEMPLATE_TYPE_NAME "gene::Genome"
#define S11N_TEMPLATE_TYPE_PROXY gene::Genome_s11n
#include <s11n.net/s11n/proxy/reg_s11n_traits_template2.hpp>
////////////////////////////////////////////////////////////////////////

#endif // gene_S11N_HPP_INCLUDED

