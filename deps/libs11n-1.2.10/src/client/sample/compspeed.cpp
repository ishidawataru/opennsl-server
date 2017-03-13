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
////////////////////////////////////////////////////////////////////////


#define CONTAINER_TYPE_COUNT 4

/***
    Below is are timings for compiling and linking this file on my box
    (1.4GHz PC w/ standard RAM and HDD) running gcc 4.1.2 under Linux,
    using libs11n 1.2.6. Each build was run several times, and the low
    and high times are shown. Each increment of COUNTAINER_TYPE_COUNT
    adds one new standard container plus possibly one POD:

    CONTAINER_TYPE_COUNT 0 == 0.809s - 0.819s
    w/o linking: 0.70s

    CONTAINER_TYPE_COUNT 1 == 1.78s - 1.85s (+pod_double proxy)
    w/o linking: 1.69s

    CONTAINER_TYPE_COUNT 2 == 2.29s - 2.31s
    w/o linking: 2.12s

    CONTAINER_TYPE_COUNT 3 == 3.19s - 3.29s  (+pod_string proxy)
    w/o linking: 3.06s

    CONTAINER_TYPE_COUNT 4 == 3.56s - 3.60s
    w/o linking: 3.33s

    Approximately 0.03s of each time is build-process overhead.

    This is no particular reason why registration of types other than
    the standard containers would take a different amount of time than
    registering the containers does. The standard containers are used
    here because those proxies ship with the library and i use them
    often in my client code.

*/

#if (CONTAINER_TYPE_COUNT > 0)
#include <s11n.net/s11n/proxy/pod/double.hpp>
#include <s11n.net/s11n/proxy/std/valarray.hpp>
#endif

#if (CONTAINER_TYPE_COUNT > 1)
#include <s11n.net/s11n/proxy/std/list.hpp>
#endif

#if (CONTAINER_TYPE_COUNT > 2)
#include <s11n.net/s11n/proxy/pod/string.hpp>
#include <s11n.net/s11n/proxy/std/map.hpp>
#endif

#if (CONTAINER_TYPE_COUNT > 3)
#include <s11n.net/s11n/proxy/std/vector.hpp>
#endif


void test_buildspeed()
{
#if (CONTAINER_TYPE_COUNT > 0)
        typedef std::valarray<double> VA;
	VA va;
        s11nlite::save( va, std::cout );
#endif

#if (CONTAINER_TYPE_COUNT > 1)
        typedef std::list<double> LT;
	LT lt;
        s11nlite::save( lt, std::cout );
#endif

#if (CONTAINER_TYPE_COUNT > 2)
        typedef std::map<double,std::string> MT;
	MT mt;
        s11nlite::save( mt, std::cout );
#endif

#if (CONTAINER_TYPE_COUNT > 3)
        typedef std::vector<double> VT;
	VT vt;
        s11nlite::save( vt, std::cout );
#endif


}

int
main( int argc, char **argv )
{
 	using namespace s11n::debug;
 	trace_mask( trace_mask() | TRACE_INFO | TRACE_TRIVIAL );
        s11nlite::serializer_class( "simplexml" );

	try
	{
		test_buildspeed();
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
        return 0;
}
