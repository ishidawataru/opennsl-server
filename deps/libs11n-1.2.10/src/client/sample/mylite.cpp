////////////////////////////////////////////////////////////////////////
// A test of the cleanup_functor concept for s11n.
// Author: stephan@s11n.net
// License: Do As You Damned Well Please
////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#  undef NDEBUG // we always want assert() to work
#endif

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>


////////////////////////////////////////////////////////////////////////
#include <s11n.net/s11n/s11nlite.hpp> // s11n & s11nlite frameworks
#include <s11n.net/s11n/micro_api.hpp>
#include <s11n.net/s11n/client_api.hpp>
// #include <s11n.net/s11n/proxy/std/map.hpp>
// #include <s11n.net/s11n/proxy/std/list.hpp>
// #include <s11n.net/s11n/proxy/pod/int.hpp>
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// misc util stuff
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
////////////////////////////////////////////////////////////////////////

class my_api : public s11nlite::client_interface
{
public:
// 	using s11nlite::client_interface serializer_interface;
// 	using s11nlite::client_interface node_type;
	my_api()
	{
	}

	virtual ~my_api()
	{
	}

	virtual bool save( const node_type & src, std::ostream & dest )
	{
		CERR << "my_api::save(node,ostream)\n";
		return this->s11nlite::client_interface::save( src, dest );
	}
	virtual bool save( const node_type & src, std::string & dest )
	{
		CERR << "my_api::save(node,string["<<dest<<"])\n";
		return this->s11nlite::client_interface::save( src, dest );
	}
	virtual node_type * load_node( const std::string & src )
	{
		CERR << "my_api::load(string["<<src<<"])\n";
		return this->s11nlite::client_interface::load_node( src );
	}
	virtual node_type * load_node( std::istream & src )
	{
		CERR << "my_api::load(istream)\n";
		return this->s11nlite::client_interface::load_node( src );
	}
};

#include <s11n.net/s11n/proxy/std/valarray.hpp>
void test_valarray()
{

        typedef std::valarray<double> VA;

        size_t sz = 10;
        VA v(sz);
        for( size_t i = 0; i < sz; i++ )
        {
                v[i] = (1+i) * i;
        }

	v *= 10.0;

	typedef s11nlite::micro_api<VA> MVA;
	MVA micro("parens");
	
        if( ! micro.buffer(v) )
	{
		throw s11n::s11n_exception( "%s:%d: Serializing valarray failed!", __FILE__,__LINE__ );
	}

	using namespace s11nlite;

        CERR << "Serialized valarray:\n" << micro.buffer() << "\n";
        std::istringstream is( micro.buffer() );
        s11n::cleanup_ptr<VA> dev( micro.load( is ) );
        if( ! dev.get() ) throw s11n::s11n_exception( "%s:%d: could not deserialize valarray<> type!", __FILE__,__LINE__ );
        CERR << "Deserialized valarray:\n";
        micro.save( *dev, std::cout );
	
	CERR << "The following calls will routed through the s11nlite add-on we installed:\n";
        std::istringstream is2( micro.buffer() );
	// save( v, std::cout );
	if( ! save_nullary_f( v, std::cout )() )
	{
		throw s11n::s11n_exception( "%s:%d: save_nullary_f() failed :(", __FILE__,__LINE__);
	}
        s11n::cleanup_ptr<VA> dev2( load_serializable<VA>( is2 ) );
	if( ! dev2.get() ) throw s11n::s11n_exception( "%s:%d: Deser of valarray failed!", __FILE__,__LINE__ );
	//s11nlite::save( *dev2, std::cout );
	save_stream_unary_f bob( std::cout );
	bob(v);
	
}

#include <s11n.net/s11n/proxy/std/list.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>
void funky_save()
{
	using namespace s11nlite;
	using namespace std;
	typedef list<string> LS;
	LS list;
	list.push_back( "boo" );
	list.push_back( "far" );
	list.push_back( "foo" );
	list.push_back( "bar" );
	for_each( list.begin(),
		  list.end(),
		  save_stream_unary_f(cout) );

	if( ! save_binary_f()( list, cout ) )
	{
		throw s11n::s11n_exception( "%s:%d: safe_binary_f() failed :(", __FILE__,__LINE__);
	}

}

int
main( int argc, char **argv )
{
 	using namespace s11n::debug;
 	trace_mask( trace_mask() | TRACE_CLEANUP );
        s11nlite::serializer_class( "simplexml" );

	try
	{
		my_api my;
		s11nlite::instance( &my );
		test_valarray();
		funky_save();
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
        return 0;
}
