/************************************************************************
Some test and demo code for s11n 1.1/1.2.

************************************************************************/
#include <stdexcept> // runtime_error
#include <memory> // auto_ptr

#include <iostream>
#include <iterator>
#include <algorithm>

#include <s11n.net/s11n/s11n.hpp> // core API
#include <s11n.net/s11n/variant.hpp> // helper for lexical casting
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // exactly what it says

#include <s11n.net/s11n/proxy/pod/int.hpp> // proxy for INT
#include <s11n.net/s11n/proxy/pod/string.hpp> // proxy for STRING
#include <s11n.net/s11n/proxy/std/map.hpp> // proxy for MAP<>
#include <s11n.net/s11n/proxy/std/list.hpp> // proxy for LIST<>

#include <s11n.net/s11n/io/data_node_io.hpp>
#include <s11n.net/s11n/io/serializers.hpp>

#include <s11n.net/s11n/s11nlite.hpp> // s11nlite, of course
#include <s11n.net/s11n/client_api.hpp> // 1.1-style s11nlite back-end class

#define NODE_TYPE s11n::s11n_node
#define NODE_TRAITS s11n::node_traits< NODE_TYPE >

typedef s11nlite::client_api<NODE_TYPE> LiteAPI;

class MyType
{
public:
	MyType() : astring("Some string")
	{
		
	}

	virtual ~MyType(){}

	virtual bool operator()( NODE_TYPE & dest ) const
	{
		CERR << "MyType::serialize()\n";
		NODE_TRAITS::set( dest, "my_value", astring );
		return true;
	}

	virtual bool operator()( const NODE_TYPE & src )
	{
		CERR << "MyType::deserialize()\n";
		if( ! NODE_TRAITS::is_set( src, "my_value" ) )
		{
			return false;
		}
		this->astring = NODE_TRAITS::get( src, "my_value", std::string(":`(") );
		return true;
	}

	std::string astring;
};

#define S11N_TYPE MyType
#define S11N_TYPE_NAME "MyType"
#include <s11n.net/s11n/reg_s11n_traits.hpp>

void load_serializers()
{
//  doesn't do anything right now because DLL support was removed
	typedef ::s11n::fac::factory_mgr<LiteAPI::serializer_interface> FacMgr;
	const std::string dfltser = "s11n::io::funxml_serializer";
	if( ! FacMgr::instance().provides( dfltser ) );
	{
		CERR << "SERIALIZERS NOT REGISTERED!\n";
	}
}


/**
   Saves a Serializable to cout, optionally using the specified
   serializer class.

   ST must be a Serializable Type.
*/
template <typename ST> 
void save_cout( const ST & s, const std::string & serclass = "parens" )
{
	using namespace s11n;
	typedef LiteAPI::serializer_interface SerT;
	typename LiteAPI::node_type n;
	LiteAPI & lite = LiteAPI::instance();
 	if( ! lite.provides_serializer( serclass ) )
 	{
 		CERR << "save_cout(): it seems that serializer '"<<serclass<<"' can't be loaded.\n";
		S11N_THROW( "Loading Serializer failed: "+serclass );
 		return;
 	}
	typedef std::auto_ptr<SerT> AP;
	AP ser = AP( lite.create_serializer( serclass ) );
	if( ! ser.get() )
	{
		CERR << "Error loading '"<<serclass<<"' serializer :(.\n";
		return;
	}
	if( ! lite.serialize( n, s ) )
	{
		CERR << "serialize(node,serializable) failed :(\n";
		return;
	}
	ser->serialize( n, std::cout );
}

void do_something()
{
	using namespace s11n;
	typedef NODE_TRAITS NTR;
	NODE_TYPE n;
	std::string val = "some value";
	bool worked = serialize( n, val );
	if( ! worked ) S11N_THROW( "serialize (string) failed!" );
	CERR << "serialize string worked? == " << worked<<"\n";
	CERR << "class name? == " << NTR::class_name(n)<<"\n";

	MyType my;
	my.astring = "this is a string";
	n.clear();
	worked = serialize( n, my );
	if( ! worked ) S11N_THROW( "serialize (MyType) failed!" );
	CERR << "serialize MyType worked? == " << worked << "\n";
	CERR << "class name? == " << NTR::class_name(n)<<"\n";
// 	dump_node_debug(n, std::cout );

	MyType * your = deserialize<NODE_TYPE,MyType>( n );
	CERR << "deserialized (MyType *) ? == " << std::hex << your << "\n";
	if( your )
	{
		CERR << "your->astring = " << your->astring<<"\n";
		save_cout( *your );
		delete your;
	}
	else
	{
		S11N_THROW( "Deserialize (MyType *) failed!" );
	}

}

#define S11N_TYPE std::map<std::string,std::string>
#define S11N_TYPE_NAME "streamable_map"
#define S11N_SERIALIZE_FUNCTOR s11n::map::streamable_map_serializable_proxy
#include <s11n.net/s11n/reg_s11n_traits.hpp>

void do_something_else()
{
	using namespace s11n;
	typedef s11n::Detail::variant variant;
	NODE_TYPE n;
	typedef node_traits<NODE_TYPE> NTR;
	typedef std::map<std::string,std::string> MapT;
	CERR << "class_name<MapT> == " << s11n_traits<MapT>::class_name(0) << "\n";
	//::s11n::cl::classloader_register_base<MapT>( "map" );

	MapT map;
	for( int i = 0; i < 10; i++ )
	{
		map["item_"+variant(i).str()] = std::string("Item #")+variant(i).str();
	}
	bool worked = serialize( n, map );
	if( ! worked ) S11N_THROW( "do_something_else(): serialize (MapT) failed!" );
	CERR << "serialize map<int,string> worked? == " << worked<<"\n";
	CERR << "class name? == " << NTR::class_name(n)<<"\n";

	map.clear();
	worked = deserialize<NODE_TYPE,MapT>( n, map );
	if( ! worked ) S11N_THROW( "do_something_else(): deserialize (MapT) failed!" );
	CERR << "deserialized (MapT&) ?? == " << worked << "\n";
	CERR << "map.size() == " << map.size()<<"\n";

	save_cout( map );

	MapT * m2 = deserialize<NODE_TYPE,MapT>( n );
	CERR << "deserialized (MapT*) ?? == " << std::hex << m2 << "\n";
	if( m2 ) CERR << "m2->size() == " << m2->size()<<"\n";
	else S11N_THROW( "do_something_else(): deserialize (MapT*) failed!" );
	delete m2;
	m2 = 0;

	std::string infile = "in.sxml";
	NODE_TYPE * n2 = s11n::io::load_node<NODE_TYPE>( infile );
	if( n2 )
	{
		CERR << "Loaded from " << infile<<":\n";
		//dump_node_debug(*n2, std::cout );
		m2 = deserialize<NODE_TYPE,MapT>( *n2 );
		delete n2;
		CERR << "deserialized (MapT *) == " << std::dec << m2 << "\n";
		if( ! m2 ) {
			CERR << "RE-DESER TO POINTER FAILED!\n";
			S11N_THROW( "deserialize (MapT*) failed!" );
		}
		else
		{
			save_cout( *m2 );
			delete m2;
		}
	}
	else
	{
		S11N_THROW( "load of test file "+infile+" failed!" );
	}


}

void do_lists()
{
	using namespace s11n;
	typedef s11n::Detail::variant variant;
	NODE_TYPE n;
	typedef node_traits<NODE_TYPE> NTR;
	typedef std::list<std::string> ListT;
	CERR << "class_name<ListT> == " << s11n_traits<ListT>::class_name(0) << "\n";
	//::s11n::cl::classloader_register_base<ListT>( "list" );

	ListT list;
	for( int i = 0; i < 10; i++ )
	{
		list.push_back( std::string("Item #")+variant(i).str() );
	}
	bool worked = serialize( n, list );
	if( ! worked ) S11N_THROW( "serialize (node,ListT&) failed!" );
	CERR << "serialize list<string> worked? == " << worked<<"\n";
	CERR << "class name? == " << NTR::class_name(n)<<"\n";

	list.clear();
	worked = deserialize( n, list );
	if( ! worked ) S11N_THROW( "Deserialize (node,ListT&) failed!" );
	CERR << "deserialized (ListT &) ?? == " << worked << "\n";
	CERR << "list.size() == " << list.size()<<"\n";

	std::auto_ptr<ListT> l2( deserialize<NODE_TYPE,ListT>( n ) );
	if( ! l2.get() ) S11N_THROW( "Deserialize (ListT*) failed!" );
	CERR << "deserialized (ListT *) ?? == " << std::hex << l2.get() << "\n";
	CERR << "l2->size() == " << l2->size()<<"\n";
}


void nested_containers()
{
	using namespace s11n;
	typedef s11n::Detail::variant variant;
	typedef std::list<std::string> SList;
	typedef std::map<std::string,SList> MapT;
	CERR << "class_name<MapT> == " << s11n_traits<MapT>::class_name(0) << "\n";

	// populate a map with some content
	MapT map;
	for( int m = 0; m < 3; m++ )
	{
		SList & sl = map["list_"+variant(m).str()];
		for( int i = 0; i < 3; i++ )
		{
			sl.push_back( std::string("Item #")+variant(i).str() );
		}
	}

	CERR << "Nested container: map<string,list<string>>...\n";
	LiteAPI & lite = LiteAPI::instance();
	lite.serializer_class( "simplexml");
	
	// serialize it...

	// directly to a stream and to an intermediate node...
 	NODE_TYPE node;
	if( ! lite.serialize( node, map ) )
	{
		S11N_THROW( "serialize or save (MapT) failed!" );
	}
	lite.save( node, std::cout );
	
	// deserialize a copy from the intermediatary node...
	std::auto_ptr<MapT> demap( lite.deserialize<MapT>( node ) );
	if( ! demap.get() )
	{
		S11N_THROW( "Deserialize (MapT*) failed!" );
	}
	CERR << "Re-deserialized:\n";
	lite.save( *demap, std::cout );

}

void show_serializers()
{
	typedef std::list<std::string> SList;
	SList sl;
	s11n::io::serializer_list<NODE_TYPE>( sl, true );
	CERR << "Registered Serializers (\"short names\" only):\n\t";
	std::copy( sl.begin(), sl.end(), std::ostream_iterator<std::string>( std::cout, "\n\t" ) );
	std::cout << std::endl;

	sl.clear();
	s11n::io::serializer_list<NODE_TYPE>( sl, false );
	CERR << "Registered Serializers (all aliases):\n\t";
	std::copy( sl.begin(), sl.end(), std::ostream_iterator<std::string>( std::cout, "\n\t" ) );

}


int main( int argc, char ** argv )
{
	CERR << "Hi!\n";
	try
	{
		show_serializers();
		do_something();
 		do_lists();
		do_something_else();
		nested_containers();
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
	return 0;
}

