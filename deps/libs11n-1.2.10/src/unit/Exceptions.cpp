#include "UnitTest.hpp"

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/s11n_debuggering_macros.hpp>

#include <s11n.net/s11n/proxy/pod/string.hpp>


class Exceptions : public UnitTest
{
public:
	Exceptions() : UnitTest("Exceptions") {}
	virtual ~Exceptions() {}
	virtual bool run()
	{
		std::string control( "a b c 42" );
		std::string check;
		{
			s11n::s11n_exception ex( "a %s c %d", "b", 42 );
			check = ex.what();
			//UT_COUT << "ex.what() == "<<ex.what()<<" [control="<<control<<"]\n";
			UT_ASSERT( ( (check==control) && "s11n_exception string formatting failed!") );
		}
		{
			s11n::factory_exception ex( "a %c c %d", (char)98, 42 );
			//UT_COUT << "ex.what() == "<<ex.what()<<" [control="<<control<<"]\n";
			check = ex.what();
			UT_ASSERT( ( (check==control) && "factory_exception string formatting failed!") );
		}
		{
			s11n::io_exception ex( "a %s c %d", "b", 42 );
			//UT_COUT << "ex.what() == "<<ex.what()<<" [control="<<control<<"]\n";
			check = ex.what();
			UT_ASSERT( ( (check==control) && "factory_exception string formatting failed!") );
		}
		return true;
	}
};

#define UNIT_TEST Exceptions
#define UNIT_TEST_NAME "Exceptions"
#include "RegisterUnitTest.hpp"
