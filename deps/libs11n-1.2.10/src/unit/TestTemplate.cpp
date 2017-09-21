#include "UnitTest.hpp"

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/s11n_debuggering_macros.hpp>

#include <s11n.net/s11n/proxy/pod/string.hpp>


class TestTemplate : public UnitTest
{
public:
	TestTemplate() : UnitTest("TestTemplate") {}
	virtual ~TestTemplate() {}
	virtual bool run()
	{
		UT_ASSERT( ("Add your test code here!" && 0) );
		return true;
	}
};

#define UNIT_TEST TestTemplate
#define UNIT_TEST_NAME "TestTemplate"
#include "RegisterUnitTest.hpp"
