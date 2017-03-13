#include "UnitTest.hpp"

#include <s11n.net/s11n/s11nlite.hpp>

#include <s11n.net/s11n/proxy/std/map.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>
#include <s11n.net/s11n/proxy/pod/double.hpp>


class Casts : public UnitTest
{
public:
	Casts() : UnitTest("Casts") {}
	virtual ~Casts() {}
	virtual bool run()
	{

		typedef std::map<int,double> MapT;
		MapT m1;
		m1[0] = -0.0;
		m1[1] = -1.0;
		m1[2] = -2.0;

		MapT * m2 = new MapT;
		{
			s11n::cleanup_ptr<MapT> m2c( m2 );
			UT_ASSERT( (s11nlite::s11n_cast( m1, *m2c )) );
		}
		return true;
	}
};

#define UNIT_TEST Casts
#define UNIT_TEST_NAME "Casts"
#include "RegisterUnitTest.hpp"
