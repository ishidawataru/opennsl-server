

#include "UnitTest.hpp"

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/micro_api.hpp>

#include <s11n.net/s11n/proxy/std/list.hpp>
#include <s11n.net/s11n/proxy/std/set.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>


class ListsTest : public UnitTest
{
public:
	typedef std::set<int> SetT;
	typedef std::list<SetT> ListT;
	ListsTest() : UnitTest("ListsTest") {}
	virtual ~ListsTest() {}
	virtual bool run()
	{
		int at = 0;
		ListT list;
		for( int li = 0; li < 3; ++li )
		{
			SetT set;
			int to = at + 5;
			for( int si = at; si < to; ++si, ++at )
			{
				set.insert( at );
			}
			list.push_back(set);
		}

		std::string tfile(this->className()+".s11n");
		s11nlite::micro_api<ListT> mic;
		UT_ASSERT(( mic.save( list, tfile ) ));

		s11n::cleanup_ptr<ListT> delist( mic.load( tfile ) );
		UT_ASSERT(( delist.get() ));

		return true;
	}
};

#define UNIT_TEST ListsTest
#define UNIT_TEST_NAME "ListsTest"
#include "RegisterUnitTest.hpp"
