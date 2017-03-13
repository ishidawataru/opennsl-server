#ifndef s11n_UNITTEST_HPP_INCLUDED
#define s11n_UNITTEST_HPP_INCLUDED 1

#include <string>
#include <stdexcept>
#include <s11n.net/s11n/factory.hpp>
#include <s11n.net/s11n/plugin/plugin.hpp>
#include <s11n.net/s11n/variant.hpp>

#include <iostream>

/**
   UT_COUT and UT_CERR are handy replacements for std::cout/cerr.
*/
#define UT_COUT std::cout << __FILE__ << ':'<<__LINE__<<": "
#define UT_CERR std::cerr << __FILE__ << ':'<<__LINE__<<": "

/**
   UnitTest is the base-most class of all s11n unit tests.
 */
class UnitTest
{
public:
	virtual ~UnitTest() {}

	/**
	   May run arbitrary logic. Should return true on success and
	   and return false or throw a std::exception on test
	   failure.

	   By convention, tests should not output anything to
	   stdout/stderr except warning or error messages. We go by
	   the policy "no news is good news", to make the output of
	   the unit tests easier to follow.

	*/
	virtual bool run() = 0;

	std::string className() const { return this->m_class; }

	/**
	   Tries to classload the given test and run it. Throws an
	   exception if it cannot load the named class, otherwise
	   returns the result of testobject->run().
	*/
	static bool run( std::string const & classname );

	/**
	   Returns a stream which points nowhere. The stream simply
	   discards all input immediately. This is often useful as an
	   argument to s11nlite::save().
	*/
	static std::ostream & nullout();

protected:
	/** Subclasses should pass their class name to this ctor. */
	UnitTest( std::string const & classname ) : m_class(classname)
	{}

	/** Sets this object's class name. */
	void className( std::string  const & c ) { this->m_class = c; }
private:
	std::string m_class;
};

/**
   The UT_ASSERT(EXPR) macro throws an exception if EXPR is false. The
   file name and line number are included in the exception string, as
   is the stringified form of EXPR. If EXPR contains commas, be sure
   to double-parenthesize it, like so:

   UT_ASSERT((some_func(arg1,arg2));

   You can include a descriptive string like so:

   UT_ASSERT( ( (my expression) && "If expression fails, this is shown." ) );
*/
#define UT_ASSERT(EXPR) if( ! (EXPR) ) \
		throw std::runtime_error( (std::string(__FILE__)+":"+s11n::Detail::variant(__LINE__).str()+\
					   ": Assertion failed: "+ # EXPR).c_str() );

/**
   UnitFactory is the factory type used to classload tests.

   Intructions for registering with the factory are in the file
   RegisterUnitTest.hpp.
*/
typedef ::s11n::fac::factory_mgr<UnitTest,std::string> UnitFactory;

#define S11N_FACREG_TYPE UnitTest
#define S11N_FACREG_TYPE_NAME "UnitTest"
#define S11N_FACREG_TYPE_IS_ABSTRACT
#include <s11n.net/s11n/factory_reg.hpp> 

/**
   NoopTest does nothing useful except that by loading it we
   demonstrate that the factory layer is working as expected.
*/
class NoopTest : public UnitTest
{
public:
	NoopTest();
	virtual ~NoopTest();
	virtual bool run();
};


#endif // s11n_UNITTEST_HPP_INCLUDED
