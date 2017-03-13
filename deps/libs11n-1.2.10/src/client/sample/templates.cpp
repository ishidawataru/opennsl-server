////////////////////////////////////////////////////////////////////////
// Demonstration file for libs11n showing a serializable class
// template.

#include <sstream>
#include <iostream>
#include <memory>

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/algo.hpp>
#include <s11n.net/s11n/type_traits.hpp>
#include <s11n.net/s11n/s11n_debuggering_macros.hpp>


// Register some types we will use with ClassTemplate:
#include <s11n.net/s11n/proxy/pod/string.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>

/**
   T1 and T2 must be Serializables. They may be pointer types, but this
   type does not own any pointers it holds.
*/
template <typename T1, typename T2>
class ClassTemplate
{
private:
	ClassTemplate( const ClassTemplate & );
	ClassTemplate & operator=( const ClassTemplate & );
public:
	ClassTemplate() : first(), second() {}

	typedef T1 first_type;
	typedef T2 second_type;

	first_type first;
	second_type second;

	friend class std::pair<first_type,second_type>;
	// friend class std::pair<first_type,second_type>;

	virtual ~ClassTemplate() {}

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		typedef s11nlite::node_traits NT;
		if( ! s11nlite::serialize_subnode<first_type>( dest, "first", this->first ) )
		{
			throw ::s11n::s11n_exception( "ClassTemplate.first serialize failed!" );
		}
		if( ! s11nlite::serialize_subnode<second_type>( dest, "second", this->second ) )
		{
			throw ::s11n::s11n_exception( "ClassTemplate.second serialize failed!" );
		}
		return true;
	}

	virtual bool operator()( const s11nlite::node_type & src )
	{
		typedef s11nlite::node_traits TR;
		typedef s11nlite::node_type NT;

		// The template mess below comes about from having to
		// be very careful with our template typing, so that
		// T1 and T2 can be pointers or non-pointers with this
		// code.

		// ACHTUNG: for (T1*) or (T2*) this function will, on
		// success, create new objects which need to be cleaned
		// up somewhere. This object doesn't officially own them.


		// Strip any pointer parts from first/second types (important in a moment):
		typedef typename ::s11n::type_traits<T1>::type first_basic_type;
		typedef typename ::s11n::type_traits<T2>::type second_basic_type;

		// We will need to manually search src for the subnodes we want:
    		const s11nlite::node_type * ch = 0;
		if( ! (ch = ::s11n::find_child_by_name( src, "first" )) ) return false;

		// Be very explicit about which deser algo we want to go through for .first:
    		if( ! s11n::deserialize<NT,first_basic_type>( *ch, this->first ) )
		{
			throw ::s11n::s11n_exception( "ClassTemplate.first deserialize failed!" );
		}

		// 'clean' does nothing for reference types and calls
		// delete on pointers. We can use this to deallocate
		// on failure.
		s11n::cleaner_upper clean;

		// Repeat for .second, taking care to deallocate .first if deser fails:
		try
		{
			if( ! (ch = ::s11n::find_child_by_name( src, "second" )) )
			{
				clean( this->first );
				return false;
			}
		}
		catch(...)
		{
				clean( this->first );
				throw;
		}
		try
		{
			if( ! s11n::deserialize<NT,second_basic_type>( *ch, this->second ) )
			{
				throw ::s11n::s11n_exception( "%s:%d: ClassTemplate.second deserialize failed!",__FILE__,__LINE__ );
			}
		}
		catch(...)
		{
			clean( this->first );
			clean( this->second ); // might not have been alloc'd
			throw;
		}
		return true;
	}
};

namespace s11n {

	/**
	   Install a cleanup functor for ClassTemplate<>, so that s11n
	   can clean these objects up for us.

	   T1 and T2 may be pointer-qualified.
	*/
	template <typename T1, typename T2>
	struct default_cleanup_functor< ClassTemplate<T1,T2> >
	{
		typedef ClassTemplate<T1,T2> cleaned_type;

		void operator()( cleaned_type & c )
		{
			typedef typename ::s11n::type_traits<T1>::type _T1;
			typedef typename ::s11n::type_traits<T2>::type _T2;
			::s11n::cleanup_serializable<_T1>( c.first );
			::s11n::cleanup_serializable<_T2>( c.second );
		}
	};

} // namespace s11n

// Register ClassTemplate
#define S11N_TEMPLATE_TYPE ClassTemplate
#define S11N_TEMPLATE_TYPE_NAME "ClassTemplate"
#define S11N_TEMPLATE_TYPE_PROXY ::s11n::default_serialize_functor
#include <s11n.net/s11n/proxy/reg_s11n_traits_template2.hpp>


int
main(int argc, char **argv)
{
 	using namespace s11n::debug;
 	trace_mask( trace_mask() | TRACE_CLEANUP ); // TRACE_INFO | TRACE_TRIVIAL );

	try
	{

		int * intp = s11n::cl::classload<int>("int");
		CERR << "int pointer = " << std::hex << intp << "\n";
		CERR << "int class_name() == " << s11n::s11n_traits<int>::class_name(0)<<"\n";
		delete intp;

		typedef ClassTemplate<int,std::string> CTIS;

		CTIS c1;
		c1.first = 17;
		c1.second = "-42";
		CERR << "c1 ==\n";
		s11nlite::save( c1, std::cout );

		typedef ClassTemplate<std::string,int> CTSI;
		CTSI c2;
		s11nlite::s11n_cast( c1, c2 );
		CERR << "c2 == (swapped T1/T2)\n";
		s11nlite::save( c2, std::cout );
		s11nlite::node_type node;

		s11nlite::serialize( node, c2 );

 		CTIS * c3 = 0;
  		c3 = s11nlite::deserialize<CTIS>( node );
 		CERR << "c3 == (deser'd c1) " << std::hex << c3 << "\n";
		if( c3 )
		{
			s11nlite::save( *c3, std::cout );
			delete c3;
			c3 = 0;
		}

 		s11n::deserialize<s11nlite::node_type,CTIS>( node, c3 );
 		if( c3 )
 		{
 			CERR << "c3 deser'd to ref-to-pointer:\n";
 			s11nlite::save( *c3, std::cout );
 			delete c3;
 			c3 = 0;
 		}

		

 		CERR << "Now try some pointer variants:\n";
 		typedef ClassTemplate<int *,std::string *> CTPIPS;
 		CTPIPS cp;
 		int myint = 7;
 		std::string mystr = "fred";
 		cp.first = &myint;
 		cp.second = &mystr;
 		s11nlite::node_traits::clear(node);
 		s11nlite::serialize( node, cp );
 		s11nlite::save( node, std::cout );
 		myint = 42;
 		mystr = "replaced";
		CERR << "After modifying shared state:\n";
 		s11nlite::save( cp, std::cout );
 		s11n::cleanup_ptr<CTPIPS> cp2( s11nlite::deserialize<CTPIPS>( node ) );
		if( ! cp2.get() )
		{
			CERR << "AAArrrggg: couldn't deser cp2!\n";
			return 3;
		}
		CERR << "A deser'd copy of our original, containing new pointers:\n";
 		s11nlite::save( *cp2, std::cout );
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
	return 0;
}
