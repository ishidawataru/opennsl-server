////////////////////////////////////////////////////////////////////////
// A test & demo app for s11n[lite].
// Author: stephan@s11n.net
// License: Do As You Damned Well Please
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory> // auto_ptr
#include <stdexcept>


////////////////////////////////////////////////////////////////////////
#include <s11n.net/s11n/s11nlite.hpp> // s11n & s11nlite frameworks


////////////////////////////////////////////////////////////////////////
// Proxies we'll need:

#include <s11n.net/s11n/proxy/pod/int.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>
#include <s11n.net/s11n/proxy/std/vector.hpp>

#define S11N_MAP_TYPE_PROXY ::s11n::map::serialize_streamable_map_f
#define S11N_MAP_TYPE_DESER_PROXY ::s11n::map::deserialize_streamable_map_f
#include <s11n.net/s11n/proxy/std/map.hpp>
// ^^^^ those #defines are consumed and undefined by the map reg supermacro


////////////////////////////////////////////////////////////////////////
// misc util stuff
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
<<<<<<< experimental.cpp
#include <s11n.net/s11n/functional.hpp>
=======
#include <s11n.net/s11n/functional.hpp>
#include <s11n.net/s11n/micro_api.hpp>
>>>>>>> 1.2
////////////////////////////////////////////////////////////////////////


struct my_serialize_fwd
{
	mutable int count;
	template <typename NT, typename ST>
	bool operator()( NT & dest, const ST & src ) const
	{
		CERR << "Routed through our custom serialize thingie: count=" << ++this->count << "\n";
		return s11n::serialize<NT,ST>( dest, src );
	}
	template <typename NT, typename ST>
		bool operator()( const NT & src, ST & dest ) const
	{
		CERR << "Routed through our custom deserialize thingie: count=" << ++this->count << "\n";
		return s11n::deserialize<NT,ST>( src, dest );
	}
};


void do_some_tests()
{
	using namespace s11n;
	typedef s11n_node NodeT;
	typedef node_traits<NodeT> NTR;
	typedef int BogoT;

	BogoT bogo = 7;
	serializable_f<BogoT,my_serialize_fwd> sr( bogo );

	s11n_node node;
	sr( node );
	sr( node );
	ser_f( bogo )(node);

	int unbogo;
	deser_f( unbogo, sr.functor /* a copy :( */ )( node );
	/** ^^^ same as this, but won't work: const-ambiguous:
	    sr.functor( node, unbogo )

	    i don't LIKE the .functor member and may look to get rid
	    of it. Maybe it's useful, though. Dunno yet.
	 */
	s11nlite::save( unbogo, std::cout );
	node.clear();
	typedef std::vector<BogoT> BV;
	NTR::class_name( node, s11n_traits<BV>::class_name(0) );
	BV vec(0);
	for( int i = 100; i >90 ; --i ) vec.push_back(i);

	std::for_each( vec.begin(),
		       vec.end(),
		       ser_to_subnode_unary_f( node, "child" )
		       );

	CERR << "object using standard approach:\n";
	s11nlite::save( vec, std::cout );
	CERR << "object using functor approach:\n";
	s11nlite::save( node, std::cout );


	BV unvec;
	deserializable_f<BV> devec( unvec );
	/** ^^^^ This only works because the "algo" we used to
	    serialize node is structurally compatible with the default
	    vector<> proxy.
	*/
	devec( node );
	CERR << "Deserialized from functor-approach data:\n";
	s11nlite::save( unvec, std::cout );
	unvec.clear();


	// An odd, but interesting, technique for deserialization:
	std::vector<std::string> sunvec;
	std::for_each( NTR::children(node).begin(),
		       NTR::children(node).end(),
		       deser_to_outiter_f<std::string>( std::back_inserter(sunvec) ) );
	CERR << "And re-deserialized through a somewhat strange technique...\n";
	s11nlite::save( sunvec, std::cout );



	typedef std::map<int,std::string> MapT;
	MapT map;
	int at = 0;
	map[at++] = "one";
	map[at++] = "two";
	map[at++] = "three";

	node.clear();
	ser_f( map )( node );
	CERR << "Map starts out like this:\n";
	s11nlite::save( node, std::cout );

	CERR << "Map using ser_to_subnode_f():\n";
	node.clear();
	std::for_each( map.begin(),
		       map.end(),
		       ser_to_subnode_unary_f( node, "child", s11n::map::serialize_streamable_pair_f() )
		       );
	s11nlite::save( node, std::cout );

	MapT unmap;
	typedef std::pair< MapT::key_type, MapT::mapped_type > NCPair; // drop the const part of MapT::value_type.first
	std::for_each( NTR::children(node).begin(),
		       NTR::children(node).end(),
		       deser_to_outiter_f<NCPair>( std::inserter(unmap, unmap.begin()),
						    s11n::map::deserialize_streamable_pair_f() )
		       );
	CERR << "Deserialized using deser_from_subnode_f():\n";
	s11nlite::save( unmap, std::cout );

	pointer_f<MapT> pf( unmap );
	CERR << "pointer_f() == " << std::hex << pf() << ", map size="<<pf->size()<<"\n";
	pointer_f<const MapT *> pf2( &unmap );
	CERR << "pf2() == " << std::hex << pf2() << "\n";

}



void do_some_more_tests()
{
	using namespace s11n;
	typedef s11n_node NT;
	typedef node_traits<NT> NTR;


	typedef std::map<int,std::string> MapT;
	MapT map;
	int at = 0;
	map[at++] = "one";
	map[at++] = "two";
	map[at++] = "three";

	NT node;

	typedef s11n::map::serialize_streamable_map_f MapSer;
	typedef s11n::map::deserialize_streamable_map_f MapDeser;
	MapSer mapser;
	MapDeser mapdeser;

	serialize_to_subnode_f<MapSer> algo( "child" );

	if( ! ser_nullary_f( node, map, algo )() )
	{
		S11N_THROW("ser_nullary_f test failed :(");
	}
	CERR << "ser_nullary_f():\n";
	s11nlite::save( node, std::cout );

	deserialize_from_subnode_f<MapDeser> dealgo( "child" );
	MapT demap;
	if( ! deser_nullary_f( node, demap, dealgo )() )
	{
		S11N_THROW("deser_nullary_ test failed :(");
	}
	CERR << "deser_nullary_f():\n";
	s11nlite::save( demap, std::cout );

	node.clear();
	if( ! ser_nullary_f(node, demap, mapser )() )
	{
		S11N_THROW("ser_nullary_f test failed :(");
	}
	CERR << "ser_nullary_f():\n";
	s11nlite::save( node, std::cout );

	MapT unmap;
	if( ! deser_nullary_f( node, unmap, mapdeser )() )
	{
		S11N_THROW("deser_nullary_f test failed :(");
	}
	CERR << "deser_nullary_f():\n";
	s11nlite::save( unmap, std::cout );

	node.clear();
	unmap.clear();
	if( !
	    s11n::logical_and( ser_nullary_f( node, map, mapser ),
			       deser_nullary_f( node, unmap, mapdeser )
			       )()
	    )
	{
		S11N_THROW("logical_and() test failed :(" );
	}
	CERR << "s11n_cast via logical_and():\n";
	s11nlite::save( unmap, std::cout );

}

template <typename NodeT>
class node_ser_helper : public ::s11n::serialize_unary_serializable_f_tag
{
public:
	typedef NodeT node_type;
	typedef ::s11n::node_traits<NodeT> node_traits;

	node_ser_helper( node_type & n ) : m_node(n)
	{
	}

	node_type & node()
	{
		return this->m_node;
	}

	template <typename SerializableT>
	bool serialize( SerializableT const & src )
	{
		return ::s11n::serialize<node_type,SerializableT>( this->node(), src );
	}

	template <typename SerializableT>
	bool serialize_subnode( std::string const & nodename, SerializableT const & src )
	{
		return ::s11n::serialize_subnode<node_type,SerializableT>( this->node(), nodename, src );
	}

	/** Lame, but true. Avoids forcing user to explicitely std::string-ize string identifiers. */
	template <typename SerializableT>
	bool serialize_subnode( char const * nodename, SerializableT const & src )
	{
		return this->serialize_subnode<SerializableT>( std::string(nodename), src );
	}

	template <typename StreamableT>
	void set( std::string const & key, StreamableT const & val )
	{
		return node_traits::set( this->node(), key, val );
	}


private:
	node_type & m_node;
};

template <typename NodeT>
class node_deser_helper// : public ::s11n::deserialize_unary_serializable_f_tag
{
public:
	typedef NodeT node_type;
	typedef ::s11n::node_traits<NodeT> node_traits;

	node_deser_helper( node_type const & n ) : m_node(n)
	{
	}

	node_type const & node() const
	{
		return this->m_node;
	}

	template <typename SerializableT>
	SerializableT * deserialize( std::string const & nodename ) const
	{
		return ::s11n::deserialize<node_type,SerializableT>( this->node(), nodename );
	}

	template <typename SerializableT>
	SerializableT * deserialize_subnode( std::string const & nodename ) const
	{
		return ::s11n::deserialize_subnode<node_type,SerializableT>( this->node(), nodename );
	}

	/** Lame, but true. Avoids forcing user to explicitely std::string-ize string identifiers. */
	template <typename SerializableT>
	SerializableT * deserialize( char const * nodename ) const
	{
		return ::s11n::deserialize_subnode<node_type,SerializableT>( this->node(), nodename );
	}

	template <typename StreamableT>
	StreamableT get( std::string const & key, StreamableT const & default_val = StreamableT() ) const
	{
		return node_traits::get( this->node(), key, default_val );
	}


private:
	node_type const & m_node;
};


void do_even_more_tests()
{
	typedef std::map<int,std::string> MapT;
	MapT map;
	int at = 0;
	map[at++] = "value #one";
	map[at++] = "value #two";
	map[at++] = "value #three";


	typedef s11nlite::node_type NT;
	typedef node_ser_helper<NT> NHT;
	NT node;
	NHT h(node);

	h.set( "foo", 1 );
	h.set( "bar", 2 );
	h.set( "barre", "hello, world" );

	h.serialize_subnode( "mymap", map );

	s11nlite::serializer_class("parens");
	if( ! s11nlite::save( h.node(), "/dev/null" ) )
	{
		throw s11n::s11n_exception( "%s:%d: Save of MapT failed :(", __FILE__,__LINE__);
	}

	typedef node_deser_helper<NT> CNHT;
	CNHT const ch(node);
	CERR << "foo == " << ch.get<int>("foo") << '\n';
	CERR << "bar == " << ch.get<int>("bar") << '\n';
	CERR << "barre == " << ch.get<std::string>("barre") << '\n';
	s11n::cleanup_ptr<MapT> clean( ch.deserialize<MapT>("mymap") );
	if( ! clean.get() )
	{
		throw s11n::s11n_exception( "%s:%d: Deser of MapT failed :(", __FILE__,__LINE__);
	}
	if( ! s11nlite::save( *clean, std::cout ) )
	{
		throw s11n::s11n_exception( "%s:%d: Save of MapT failed :(", __FILE__,__LINE__);
	}

	return;
}

int
main( int argc, char **argv )
{
//  	using namespace s11n::debug;
//  	trace_mask( trace_mask() | TRACE_CLEANUP );
        s11nlite::serializer_class( "parens" );
	try
	{
<<<<<<< experimental.cpp
		do_some_tests();
		// do_some_more_tests();
=======
		// do_some_tests();
		// do_some_more_tests();
		do_even_more_tests();
>>>>>>> 1.2
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
	catch( ... )
	{
		CERR << "UNKNOWN EXCEPTION!!!\n";
		return 2;
	}
        return 0;
}


