////////////////////////////////////////////////////////////
// A demo of serializing arrays with s11n.
// Required linker arguments: -rdynamic -ls11n
#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/micro_api.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>
#include <s11n.net/s11n/proxy/pod/double.hpp>
#include <s11n.net/s11n/proxy/pod/char.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>
#include <s11n.net/s11n/proxy/std/vector.hpp>
#include <s11n.net/s11n/functional.hpp>

#include <s11n.net/s11n/variant.hpp>

/**
   An s11n algo for chars to allow us to deserialize integers to
   chars.
 */
struct char_s11n
{
	template <typename NodeT>
	bool operator()( NodeT & dest, char const & src ) const
	{
		typedef s11n::node_traits<NodeT> NTR;
		NTR::class_name( dest, "char" );
		NTR::set( dest, "v", src );
		return true;
	}

	template <typename NodeT>
	bool operator()( NodeT const & src, char & dest ) const
	{
		typedef s11n::node_traits<NodeT> NTR;
		s11n::Detail::variant val( NTR::get( src, "v", std::string() ) );
		if( val.str().empty() ) return false;
		if( 1 == val.str().size() )
		{
			dest = val.str()[0];
			return true;
		}
		// assume it's an integer...
		char c = val.template cast_to<int>(0);
		if( 0 == c ) return false;
		dest = c;
		return true;
	}
};

// #define S11N_TYPE char
// #define S11N_TYPE_NAME "char"
// #define S11N_SERIALIZE_FUNCTOR char_s11n
// #include <s11n.net/s11n/reg_s11n_traits.hpp>


void do_array()
{
	const int asz = 10;
	typedef int ArT;
	ArT arI[10] = {
	65, 66, 67, 68, 69,
	97, 98, 99, 100, 101
	};

	s11nlite::node_type node;
	using namespace s11n;
	std::for_each( arI, arI+asz, ser_to_subnode_unary_f( node, "number" ) );

	if( ! s11nlite::save( node, std::cout ) )
	{
		throw s11n::s11n_exception( "%s:%d: save() of array failed.", __FILE__,__LINE__ );
	}

// 	ArT darI[10];

	s11nlite::node_type denode;
	typedef s11nlite::node_traits NTR;
	// typedef char VdT;
	std::vector<char> vec;
	std::for_each( NTR::children(node).begin(),
		       NTR::children(node).end(),
		       deser_to_outiter_f<int>( std::back_inserter(vec) ) );
	if( ! s11nlite::save( vec, std::cout ) )
	{
		throw s11n::s11n_exception( "%s:%d: save() of vec failed.", __FILE__,__LINE__ );
	}
	

}

int
main( int argc, char **argv )
{
	std::string format = (argc>1) ? argv[1] : "parens";
	s11nlite::serializer_class( format ); // define output format
	try
	{
		do_array();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
	return 0;
}
