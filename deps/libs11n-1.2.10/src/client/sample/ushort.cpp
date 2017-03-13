////////////////////////////////////////////////////////////
// Some test/debug code for serialization of char vs. ushort.
#include <s11n.net/s11n/s11nlite.hpp>
#include <stdint.h>
//typedef unsigned short ShortType;
//typedef unsigned char ShortType;
typedef uint8_t ShortType;
#if 1
#define S11N_TYPE ShortType
#define S11N_TYPE_NAME "Short"
#define S11N_SERIALIZE_FUNCTOR s11n::streamable_type_serialization_proxy
#include <s11n.net/s11n/reg_s11n_traits.hpp>
#endif

void do_test()
{
    s11nlite::node_type denode;
    s11nlite::node_type snode;
    typedef s11nlite::node_traits NTR;

    ShortType si;
    si = 27; // ascii 27 == ESC

    if( ! s11nlite::save( si, std::cout ) )
    {
        throw s11n::s11n_exception( "%s:%d: save() of ShortType failed!", __FILE__,__LINE__ );
    }
}

int
main( int argc, char const **argv )
{
	std::string format = (argc>1) ? argv[1] : "funxml";
	s11nlite::serializer_class( format ); // define output format
	try
	{
		do_test();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
	return 0;
}
