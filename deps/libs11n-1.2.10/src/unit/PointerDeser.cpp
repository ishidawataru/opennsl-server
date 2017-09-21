#include "UnitTest.hpp"

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>


class PointerDeser : public UnitTest
{
public:
    PointerDeser() : UnitTest("PointerDeser") {}
    virtual ~PointerDeser() {}
    virtual bool run()
    {
	s11nlite::node_type node;
	std::string orig = "hello world";
	UT_ASSERT( s11n::serialize( node, orig ) );

	// Try to deserialize a copy of cp, but to a pointer:
	{
	    std::string * cp = 0;
	    std::auto_ptr<std::string> acp( cp );
	    UT_ASSERT( ((s11n::deserialize( node, cp )) && "deserialize(const node &,T*) didn't do what we want.") );
	    UT_ASSERT( cp );
	    UT_ASSERT( (s11nlite::save( *cp, UnitTest::nullout() )) );
	}

	// try a slightly different approach:
	s11n::cleanup_ptr<std::string> cl;
	UT_ASSERT( ((s11n::deserialize( node, cl )) && "deserialize(const node &,cleanup_ptr &) didn't do what we want." ));
	UT_ASSERT( (cl.get() && "deserialize to cleanup_ptr failed!") );
	UT_ASSERT( (s11nlite::save( *cl, UnitTest::nullout() ) ) );

	return true;
    }
};

#define UNIT_TEST PointerDeser
#define UNIT_TEST_NAME "PointerDeser"
#include "RegisterUnitTest.hpp"
