////////////////////////////////////////////////////////////////////////
// A very basic benchmark of the compile- and link-time costs of adding
// s11n-related type registrations.
//
// Author: stephan@s11n.net
// License: Do As You Damned Well Please
////////////////////////////////////////////////////////////////////////

#include <stdexcept>


////////////////////////////////////////////////////////////////////////
#include <s11n.net/s11n/s11nlite.hpp> // s11n & s11nlite frameworks
#include <s11n.net/s11n/micro_api.hpp>
////////////////////////////////////////////////////////////////////////


#define OBJECT_POOL_DEBUGGING 1
#include "object_pool.hpp"
#include "object_pool_s11n.hpp"
#include <s11n.net/s11n/proxy/pod/size_t.hpp> // key type of object_pool

#define S11N_LIST_TYPE_PROXY ::s11n::list::serialize_streamable_list_f
#define S11N_LIST_TYPE_DESER_PROXY ::s11n::list::deserialize_streamable_list_f
#include <s11n.net/s11n/proxy/std/list.hpp>

void test_pool()
{

	using namespace pool;
	typedef std::list<int> ListT;
	typedef object_pool<ListT> PoolT;
	PoolT pool;

	typedef s11nlite::node_traits NTR;
	typedef s11nlite::node_type NT;
	std::string cname = s11n::s11n_traits<ListT>::class_name(0);
	int at = 0;
	for( int i = 0; i < 3; i++ )
	{
		ListT * li = pool.create_object( cname );
		if( ! li ) throw std::runtime_error( "pool.create() failed!" );
		int x = at + 10;
		for( ; at < x; ++at )
		{
			li->push_back( at );
		}
	}
	s11nlite::micro_api<PoolT> mic("simplexml");
	mic.buffer( pool );
	CERR << mic.buffer() << '\n';
	
	s11n::cleanup_ptr<PoolT> cl( mic.load_buffer() );
	if( ! cl.get() )
	{
		throw s11n::s11n_exception( "%s:%d: Could not load object pool!", __FILE__, __LINE__ );
	}
	mic.save( *cl, std::cout );
	
}

int
main( int argc, char **argv )
{
#if 0
  	using namespace s11n::debug;
  	trace_mask( trace_mask() | TRACE_ALWAYS ); // WARNING | TRACE_IO | TRACE_NYI | TRACE_FACTORY );
#endif
        s11nlite::serializer_class( "simplexml" );
	try
	{
		test_pool();
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
        return 0;
}
