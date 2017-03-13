////////////////////////////////////////////////////////////////////////
// Demonstration code for s11n: show how to tell s11n to use a
// virtual className() function to set the proper name of a type during
// serialization.
//
// Author: stephan@s11n.net
// License: Do As You Damned Well Please
////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#  undef NDEBUG // we always want assert() to work
#endif

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>


////////////////////////////////////////////////////////////////////////
#include <s11n.net/s11n/s11nlite.hpp> // s11n & s11nlite frameworks
#include <s11n.net/s11n/micro_api.hpp>
#include <s11n.net/s11n/plugin/plugin.hpp>
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// misc util stuff
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
////////////////////////////////////////////////////////////////////////

struct BaseType
{
private:
	std::string m_cname;
public:
	BaseType() :m_cname("MyBaseType") {}
    virtual ~BaseType() = 0;

protected:
	BaseType( const std::string & classname ) :m_cname(classname) {}
	void className( const std::string & cn )
	{
		this->m_cname = cn;
	}
public:
	virtual std::string className() const
	{
		return this->m_cname;
	}

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
 		typedef s11nlite::node_traits NTR;
// Normally we would call:
// 		NTR::class_name( dest, this->className() );
// But this is not necessary because we will tell s11n how to use
// the className() member function to set that for us.
 		NTR::set( dest, "this_was_set_by", "base type implementation" );
		return true;
	}
	virtual bool operator()( const s11nlite::node_type & src )
	{
		//typedef s11nlite::node_traits NTR;
		return true;
	}
};

BaseType::~BaseType() {} // pure virtual dtors must still be implemented


struct SubType : public BaseType
{
	SubType() : BaseType("SubType")
	{
	}
	virtual ~SubType()
	{
	}

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		if( ! this->BaseType::operator()( dest ) ) return false;
// Normally we would now call:
// 		s11nlite::node_traits::class_name( dest, "SubType" );
// But this is not necessary because we will tell s11n how to use
// the className() member function to set that for us.
 		typedef s11nlite::node_traits NTR;
 		NTR::set( dest, "and_this_was_set_by", "subtype implementation: " +this->className() );
		return true;
	}
	virtual bool operator()( const s11nlite::node_type & src )
	{
		CERR << "SubType::deserialize\n";
		if( ! this->BaseType::operator()( src ) ) return false;
		return true;
	}

};

// This would be the normal way to register BaseType as the base
// of the Serializable hierarchy:
//   #define S11N_TYPE BaseType
//   #define S11N_TYPE_NAME "BaseType"
//   #define S11N_ABSTRACT_BASE 1
//   #include <s11n.net/s11n/reg_s11n_traits.hpp>
// Instead, we're going to hand-roll our own registration so that we
// can take advantage of BaseType::className() in s11n_traits::class_name().
namespace s11n {

        template < typename SubT >
        struct s11n_traits < SubT, BaseType >
        {
                typedef BaseType serializable_type;
                typedef ::s11n::default_serialize_functor serialize_functor;
                typedef serialize_functor deserialize_functor;
                typedef ::s11n::cl::object_factory<BaseType> factory_type;
                typedef ::s11n::default_cleanup_functor<serializable_type> cleanup_functor;
		static std::string class_name( const serializable_type * instance_hint )
		{
			return instance_hint
				? instance_hint->className()
				: "UnknownBaseType";
		}
        };

} // namespace s11n
// Register BaseType with the base classloader...
// We don't actually need to install a factory registration for abstract
// types, but we do because there is a very subtle difference:
// the factory layer can tell the difference, and might do a DLL lookup
// for our type if it doesn't have a factory, but won't do so if it finds
// a factory but the factory returns 0.
#define S11N_TYPE BaseType
#define S11N_TYPE_NAME "MyBaseType"
#define S11N_ABSTRACT_BASE
#include <s11n.net/s11n/reg_s11n_traits.hpp>

// Now we register SubType as a Serializable subtype implementing
// BaseType's Serializable interface...
#define S11N_TYPE SubType
#define S11N_TYPE_NAME "SubType"
#define S11N_BASE_TYPE BaseType
#include <s11n.net/s11n/reg_s11n_traits.hpp>


void test_one()
{
 	using namespace s11n::debug;
 	trace_mask_changer guard( trace_mask() | TRACE_CLEANUP );

	typedef s11n::cleanup_ptr<BaseType> CP;
	typedef s11nlite::micro_api<BaseType> MVA;
	MVA micro; // ("parens");
	CP s( s11n::cl::classload<BaseType>( "SubType" ) );
	if( ! s.get() ) throw s11n::s11n_exception( "%s:%d: Assertion failed: (0!=s)", __FILE__,__LINE__);
        micro.save( *s, std::cout );
}

int
main( int argc, char **argv )
{

	try
	{
		test_one();
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
	CERR << "Bye!\n";
        return 0;
}
