#include "UnitTest.hpp"

#include <s11n.net/s11n/s11nlite.hpp>

#include <s11n.net/s11n/proxy/pod/int.hpp>
#include <s11n.net/s11n/proxy/pod/double.hpp>
#include <s11n.net/s11n/proxy/pod/float.hpp>
#include <s11n.net/s11n/proxy/pod/bool.hpp>
#include <s11n.net/s11n/proxy/pod/char.hpp>

// Rememeber that size_t is likely to collide
// with (be the same type as) uint or ulong,
// but this depends on the the platform.
#include <s11n.net/s11n/proxy/pod/size_t.hpp>
// #include <s11n.net/s11n/proxy/pod/ulong.hpp>
// #include <s11n.net/s11n/proxy/pod/uint.hpp>


class PodsTest : public UnitTest
{
public:
	PodsTest() : UnitTest("PodsTest") {}
	virtual ~PodsTest() {}
	virtual bool run()
	{
		s11nlite::node_type tgt;
#define SPOD(TYPE,VAR,VAL) \
		TYPE VAR(VAL);						\
		UT_ASSERT((s11nlite::serialize_subnode( tgt, # TYPE, VAR )));

		SPOD(int,I,42);
		SPOD(size_t,ST,420000);
// 		SPOD(uint,UI,42);
// 		SPOD(ulong,UL,42);
		SPOD(double,D,42.424242);
		SPOD(float,F,0.42);
		SPOD(char,C,'*');
		SPOD(bool,B,true);
#undef SPOD
		std::string fname(this->className()+".s11n");
		UT_ASSERT((s11nlite::save(tgt,fname)));
		std::auto_ptr<s11nlite::node_type> load( s11nlite::load_node( fname ) );
		UT_ASSERT( load.get() );

#define DPOD(TYPE,VAR,VAL)							\
		s11n::cleanup_ptr<TYPE> VAR( s11nlite::deserialize_subnode<TYPE>( *load, # TYPE ) ); \
		UT_ASSERT( VAR.get() ); \
		UT_ASSERT( (*VAR == VAL) );

		DPOD(int,IP,I);
		DPOD(size_t,STP,ST);
// 		DPOD(uint,UIP,UI);
// 		DPOD(ulong,ULP,UL);
		DPOD(double,DP,D)
		DPOD(float,FP,F);
		DPOD(char,CP,C);
		DPOD(bool,BP,B);
#undef DPOD

		return true;
	}
};

#define UNIT_TEST PodsTest
#define UNIT_TEST_NAME "PodsTest"
#include "RegisterUnitTest.hpp"
