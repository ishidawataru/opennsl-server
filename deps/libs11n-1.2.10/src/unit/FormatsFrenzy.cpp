#include "UnitTest.hpp"

#include <s11n.net/s11n/s11nlite.hpp>

#include <s11n.net/s11n/proxy/std/map.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>

// Rememeber that size_t is likely to collide
// with (be the same type as) uint or ulong,
// but this depends on the the platform.
#include <s11n.net/s11n/proxy/pod/size_t.hpp>
// #include <s11n.net/s11n/proxy/pod/ulong.hpp>
// #include <s11n.net/s11n/proxy/pod/uint.hpp>


class FormatsFrenzy : public UnitTest
{
public:
	std::list<std::string> formats;
	FormatsFrenzy() : UnitTest("FormatsFrenzy") {}
	virtual ~FormatsFrenzy() {}
	virtual bool run()
	{

		typedef std::map<int,std::string> MapT;
		MapT map;
		map[0] = "zero"; map[1] = "one";
		map[2] = "two"; map[3] = "three";

		this->formats.clear();
		::s11n::io::serializer_list<s11nlite::node_type>( this->formats, true );

		s11nlite::node_type snode;
		UT_ASSERT( s11nlite::serialize( snode, map ) );
		UT_ASSERT( s11nlite::save( snode, this->className()+".reference.s11n" ) );
		typedef std::list<std::string> SList;
		SList::const_iterator b1 = this->formats.begin();
		SList::const_iterator e1 = this->formats.end();
		for( ; e1 != b1; ++b1 )
		{
			std::string f1( this->className()+"."+(*b1)+".s11n" );
			std::auto_ptr<s11nlite::serializer_interface> ser1( s11n::cl::classload<s11nlite::serializer_interface>( (*b1) ) );
			UT_ASSERT( ser1.get() );
			UT_ASSERT( ser1->serialize( snode, f1 ) );
			SList::const_iterator b2 = this->formats.begin();
			SList::const_iterator e2 = this->formats.end();
			for( ; e2 != b2; ++b2 )
			{
				//UT_COUT << "Format '"<<(*b1)<<"' ==> '"<<(*b2)<<"'...\n";
				std::auto_ptr<s11nlite::node_type> lnode( s11nlite::load_node( f1 ) );
				UT_ASSERT( lnode.get() );
				std::string f2( this->className()+"."+(*b2)+".s11n" );
				std::auto_ptr<s11nlite::serializer_interface> ser2( s11n::cl::classload<s11nlite::serializer_interface>( (*b2) ) );
				UT_ASSERT( ser2.get() );
				UT_ASSERT( ser2->serialize( *lnode, f2 ) );
			}
		}

		return true;
	}
};

#define UNIT_TEST FormatsFrenzy
#define UNIT_TEST_NAME "FormatsFrenzy"
#include "RegisterUnitTest.hpp"
