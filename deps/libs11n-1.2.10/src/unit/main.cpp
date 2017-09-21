////////////////////////////////////////////////////////////////////////
// this is the main app for the libs11n unit tests.

#include "UnitTest.hpp"
#include <memory>
#include <sstream>

/**
   A streambuffer which discards all output sent to it. Used for
   implementing a "/dev/null stream".
*/
class NullStreamBuffer : public std::streambuf
{
private:
	std::ostream * m_os;
	std::streambuf * m_old;
public:
	/**
	   Implants this object as os's rdbuf(). When this
	   object is destroyed, the previous rdbuf() is
	   restored.
	   os must both outlive this object.
	*/
	NullStreamBuffer(std::ostream & os )
		: m_os(&os), m_old(os.rdbuf())
	{
		this->setp( 0, 0 );
		this->setg( 0, 0, 0 );
		os.rdbuf( this );
	}


	/**
	   If this object was constructed with a target
	   stream, this restores that stream's rdbuf() to its
	   pre-ctor state. If no stream was assigned then
	   this function does nothing.
	*/
	virtual ~NullStreamBuffer()
	{
		if( this->m_os )
		{
			this->m_os->rdbuf( this->m_old );
		}
	}

	/**
	   Does nothing and returns c.
	*/
	virtual int overflow( int c ) { return c; }

};


std::ostream & UnitTest::nullout()
{
	static std::ostringstream os;
	static NullStreamBuffer ns( os );
	return os;
}

bool UnitTest::run( std::string const & name )
{
	UT_COUT << "Running test: " << name << std::endl;
	UnitFactory & f = UnitFactory::instance();
	if( ! f.provides(name) )
	{
		std::string dll = ::s11n::plugin::open(name);
		std::string err( ::s11n::plugin::dll_error() );
		if( dll.empty() )
		{
			UT_CERR << "Warning: open("<<name<<") failed: " << err << '\n';
		}
		if( ! f.provides(name) )
		{
			throw std::runtime_error("UnitFactory doesn't provide a test named '"+name+"'.");
		}
	}
	std::auto_ptr<UnitTest> t( f.create( name ) );
	if( ! t.get() )
	{
		throw std::runtime_error("UnitFactory.create("+name+") failed!");
	}
	return t->run();
}

NoopTest::NoopTest() : UnitTest("NoopTest") {}
NoopTest::~NoopTest() {}
bool NoopTest::run()
{
	UT_COUT << "Test " << this->className() << " does nothing.\n";
	return true;
}
#define UNIT_TEST NoopTest
#define UNIT_TEST_NAME "NoopTest"
#include "RegisterUnitTest.hpp"

#include <s11n.net/s11n/s11n_debuggering_macros.hpp>

int main( int argc, char **argv )
{
	if( 1 == argc )
	{
		UT_CERR << "Usage: " << argv[0] << " TestName1 [... TestNameN]" << std::endl;
		return 127;
	}
	try
	{
		std::string tn;
		for( int i = 1; i < argc; ++i )
		{
			tn = argv[i];
			if( ! UnitTest::run( tn ) )
			{
				throw std::runtime_error( "Test failed: "+tn );
			}
		}
	}
	catch( std::exception const & ex )
	{
		UT_CERR << "EXCEPTION: " << ex.what() << std::endl;
		return 1;
	}
	catch(...)
	{
		UT_CERR << "UNKNOWN EXCEPTION!" << std::endl;
		return 2;

	}
	return 0;
}
