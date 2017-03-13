// a simple random number generator for shellish

#include <stdlib.h> // random()
#include <time.h> // time()

#include <s11n.net/shellish/shellish.hpp>
#include <s11n.net/shellish/strtool.hpp>

#include <s11n.net/shellish/shellish_debuggering_macros.hpp> // CERR

namespace shellish {
	/**
	   Returns a random number in [min..max] using ::random(). The
	   RNG is seeded with the current time before it is used for
	   the first time. If you want to force a specific seed,
	   call this function once, THEN set your seeed, and it will
	   behave as expected.
	*/
	static unsigned long
	shellish_random( unsigned long min,
			 unsigned long max )
	{
		static bool unseeded = false;
		if( !unseeded && (unseeded=true) )
		{
			unsigned long seed = ::time(NULL);
			::srandom( seed );
		}
		if( max == min ) return min;
		if( max < min )
		{
			unsigned long tmp = max;
			max = min;
			min = tmp;
		}
		return ( min + ( ::random() % ((unsigned long) (max - min)+1) ) );
	}

	/**
	   Generates a random number between $1 and $2, inclusive. If $3 is
	   passed it generates $3 such numbers and shows both the average
	   result and grand total.
	*/
	int handle_random( const shellish::arguments & args )
	{
		std::ostream & os = ::shellish::ostream();
		long min = shellish::strtool::from<long>( args[1], -1 );
		long max = shellish::strtool::from<long>( args[2], -2 );
		if( (-1 == max) || (-1 == min) )
		{
			os << "Usage error: pass numbers as arguments $1 and $2, where $1 < $2 and both are >= 0."
			   << std::endl;
			return shellish::ErrorUsageError;
		}
		int loops = shellish::strtool::from<int>( args[3], 1 );
		double accum = 0.0;
		unsigned long rng = 0;
		for( int i = 0; i < loops; i++ )
		{
			rng = shellish_random(min,max);
			accum += rng;
			os << "random("<<min<<","<<max<<") = " <<rng<<"\n";
		}
		if( loops > 1 )
		{
			os << "Total = " << accum
			   << " Average of "<<loops<<" results = " << (double) (accum / loops)
			   << std::endl;                
		}
		return 0;
	}

        void init_shellish_random()
        {
                shellish::map_commander( "random", handle_random, "Generates a random integer between $1 and $2, $3 times." );
        }

        static int bogus_random_init = (init_shellish_random(),0);
}
