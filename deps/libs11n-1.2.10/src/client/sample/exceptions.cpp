#include <sstream>
#include <iostream>

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/proxy/std/list.hpp>
#include <s11n.net/s11n/s11n_debuggering_macros.hpp>


class Base {

#define THROW_AFTER_HOWMANY (4)
public:
	Base()
	{
	}
	virtual ~Base()
	{
		CERR << "~Base()\n";
	}
	static int counter;

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		typedef s11nlite::node_traits NT;
		NT::class_name( dest, "Base" );
		return true;
	}

	virtual bool operator()( const s11nlite::node_type & src )
	{
		CERR << "Countdown to exception: " << counter << "\n";
		--counter;
		if( counter == 0 )
		{
			counter = THROW_AFTER_HOWMANY;
			throw ::s11n::s11n_exception( "%s:%d: Testing exceptions from client-side class.", __FILE__, __LINE__ );
		}
		typedef s11nlite::node_traits TR;
		return true;
	}
};
int Base::counter = THROW_AFTER_HOWMANY;


#define S11N_TYPE Base
#define S11N_TYPE_NAME "Base"
#include <s11n.net/s11n/reg_s11n_traits.hpp>


int
main(int argc, char **argv)
{
	typedef std::list<Base *> BaseList;

	BaseList li;
	li.push_back( new Base );
	li.push_back( new Base );
	li.push_back( new Base );
	li.push_back( new Base );
	li.push_back( new Base );

	try
	{
		s11nlite::node_type node;
		if( ! s11nlite::serialize( node, li ) )
		{
			CERR << "serialize list failed :(\n";
			return 1;
		}
		s11nlite::save( node, std::cout );
 		::s11n::cleanup_serializable( li );
		CERR << "pre-deser: li.size() == " << li.size() << "\n";
		s11nlite::deserialize( node, li );
		CERR << "post-deser: li.size() == " << li.size() << "\n";
	}
	catch( const ::s11n::s11n_exception & sex )
	{
		CERR << "s11n_exception: " << sex.what() << "\n";
		return 0; // we expected this
	}
	catch( const std::exception & ex )
	{
		CERR << "std::exception: " << ex.what() << "\n";
		return 3;
	}
	catch( ... )
	{
		CERR << "UNKNOWN EXCEPTION!\n";
		return 4;
	}
	CERR << "final li.size() == " << li.size() << "\n";
	::s11n::cleanup_serializable( li );
	return 0;
}
