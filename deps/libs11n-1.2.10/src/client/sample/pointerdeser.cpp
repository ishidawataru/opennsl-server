////////////////////////////////////////////////////////////////////////
// A test & demo app for s11n[lite].
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
#include <memory> // auto_ptr
#include <stdexcept>


////////////////////////////////////////////////////////////////////////
#include <s11n.net/s11n/s11nlite.hpp> // s11n & s11nlite frameworks
#include <s11n.net/s11n/micro_api.hpp> // save/load helper

// proxies for types we will serialize:
#include <s11n.net/s11n/proxy/pod/string.hpp>
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// misc util stuff
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR, S11N_TRACE()
////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{

        s11nlite::serializer_class( "parens" );
	try
	{
		// test deserialize(const NodeT&, T * &)
		s11n::debug::trace_mask( s11n::debug::trace_mask() | s11n::debug::TRACE_CLEANUP );

		// Set up some bogus data:
		s11nlite::node_type node;
		std::string orig = "hello world";
		s11nlite::serialize( node, orig );


		// Try to deserialize a copy:
		std::string * cp = 0;
		if( ! s11n::deserialize( node, cp ) )
		{
			throw s11n::s11n_exception("%s:%d: deserialize(const node &,T*) didn't do what we want.",__FILE__,__LINE__);
		}
		s11nlite::save( *cp, std::cout );
		delete cp;
		cp = 0;

		// try a slightly different approach:
		s11n::cleanup_ptr<std::string> cl;
		CERR << "cl.get() == " << std::hex << cl.get() << '\n';
		if( ! s11n::deserialize( node, cl ) )
		{
			throw s11n::s11n_exception("%s:%d: deserialize(const node &,cleanup_ptr &) didn't do what we want.",__FILE__,__LINE__);
		}
		CERR << "cl.get() == " << std::hex << cl.get() << '\n';
		s11nlite::save( *cl, std::cout );
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
