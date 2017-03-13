#include "UnitTest.hpp"

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/micro_api.hpp>

#include <s11n.net/s11n/proxy/std/valarray.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>


class ValArray : public UnitTest
{
public:
	ValArray() : UnitTest("ValArray") {}
	virtual ~ValArray() {}
	virtual bool run()
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

		UT_ASSERT( micro.buffer(v) );
		using namespace s11nlite;
		
		std::istringstream is( micro.buffer() );
		s11n::cleanup_ptr<VA> dev( micro.load( is ) );
		UT_ASSERT( dev.get() && "could not deserialize valarray<> type!" );
		UT_ASSERT( dev->size() == v.size() );
		UT_ASSERT( micro.save( *dev, UnitTest::nullout() ) );

		UT_ASSERT( (save_nullary_f( v, UnitTest::nullout() )() ));
		std::istringstream is2( micro.buffer() );
		s11n::cleanup_ptr<VA> dev2( load_serializable<VA>( is2 ) );
		if( ! dev2.get() ) throw s11n::s11n_exception( "%s:%d: Deser of valarray failed!", __FILE__,__LINE__ );
		UT_ASSERT( dev2->size() == dev->size() );
		UT_ASSERT( (*dev2)[0] == (*dev)[0] );
		UT_ASSERT( ! ((*dev2)[sz-1] == (*dev)[0]) );
		save_stream_unary_f bob( UnitTest::nullout() );
		UT_ASSERT( bob(v) );
		return true;
	}
};

#define UNIT_TEST ValArray
#define UNIT_TEST_NAME "ValArray"
#include "RegisterUnitTest.hpp"
