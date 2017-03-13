////////////////////////////////////////////////////////////////////////
// A test of the cleanup_functor concept for s11n.
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
#include <s11n.net/s11n/proxy/std/map.hpp>
#include <s11n.net/s11n/proxy/std/list.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// misc util stuff
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
#include <s11n.net/s11n/algo.hpp> // free_list_entries()
////////////////////////////////////////////////////////////////////////


struct Type1
{
protected:
	int m_c;
	static int counter;
public:

	Type1()
	{
		this->m_c = ++counter;
	}
	virtual ~Type1() { CERR << "~Type1#"<<this->m_c<<"()\n"; }

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		typedef s11nlite::node_traits NTR;
		NTR::class_name( dest, "Type1" );
		NTR::set( dest, "number", this->m_c );
		return true;
	}
	virtual bool operator()( const s11nlite::node_type & src )
	{
		CERR << "Type1::deserialize\n";
		typedef s11nlite::node_traits NTR;
		this->m_c = NTR::get( src, "number", -1 );
		return -1 != this->m_c;
	}
};

int Type1::counter = 0;

struct Type2 : public Type1
{
	Type2(){}
	virtual ~Type2() { CERR << "~Type2#"<<this->m_c<<"()\n"; }

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		if( ! this->Type1::operator()( dest ) ) return false;
		s11nlite::node_traits::class_name( dest, "Type2" );
		return true;
	}
	virtual bool operator()( const s11nlite::node_type & src )
	{
		CERR << "Type2::deserialize\n";
		if( ! this->Type1::operator()( src ) ) return false;
// 		S11N_THROW( "This deser operator throws intentionally." );
//  		return false;
		return true;
	}

};

#define S11N_TYPE Type1
#define S11N_TYPE_NAME "Type1"
#include <s11n.net/s11n/reg_s11n_traits.hpp>

#define S11N_TYPE Type2
#define S11N_TYPE_NAME "Type2"
#define S11N_BASE_TYPE Type1
#include <s11n.net/s11n/reg_s11n_traits.hpp>

void bogo()
{
	CERR << "Structure of data file looks more or less like:\n";
	std::auto_ptr<s11nlite::node_type> nodecheck( s11nlite::load_node( "bugchase.s11n" ) );
	s11n::debug::dump_node_structure<s11nlite::node_type>( *nodecheck );
	s11nlite::save( *nodecheck, std::cout );
	std::auto_ptr<s11nlite::node_type> nodecheck2( s11nlite::load_node( "bugchase.s11n" ) );
	s11n::debug::dump_node_structure<s11nlite::node_type>( *nodecheck2 );
	s11nlite::save( *nodecheck2, std::cout );
}

int
main( int argc, char **argv )
{
// 	using namespace s11n::debug;
// 	trace_mask( trace_mask() | TRACE_INFO | TRACE_TRIVIAL );
	std::string format = "simplexml";
//  	format = "funtxt";
        s11nlite::serializer_class( format );

	try
	{
		bogo();
		return 0;
		typedef std::map<int,Type1 *> Map1;
		s11nlite::micro_api<Map1> mic1;
		Map1 * m1 = new Map1;
		for( int i = -10; i < -5; i++ )
		{
			(*m1)[i] = (0 == (i%4)) ? new Type2 : new Type1;
		}

		std::string mfile = "bugchase.s11n";
		if( ! mic1.save( *m1, mfile ) ) S11N_THROW( "Saving file "+mfile+" failed!" );
		CERR << "Saved file " << mfile << ".\n";
 		s11n::cleanup_serializable( m1 );
		CERR << "^^^ You should see several Type1 dtors BEFORE this line. "
		     << "m1 == " << std::hex << m1 << "\n";
		assert( 0 == m1 );


		Map1 * m1b = mic1.load( mfile );
		if( ! m1b )
		{
			S11N_THROW("load of file "+mfile+" failed!" );
		}
		CERR << "Loaded data file file " << mfile << ":\n";
		mic1.save( *m1b, std::cout );
		s11n::cleanup_serializable( m1b );
		CERR << "^^^ Should see more Type1/2 dtor calls.\n";
		if( 0 != m1b ) S11N_THROW("m1b should be 0 at this point!");


		CERR << "Now we do a map<int,list<Type1*> > ...\n";
		// Nested container...
		typedef std::list<Type1 *> ListT;
		ListT li;
		for( int i = 1; i < 10; i++ )
		{
			li.push_back( ::s11n::cl::classload<Type1>( (0 == (i%3))
								    ? "Type2"
								    : "Type1"
								    ) );

		}
		typedef std::map<int,ListT> MapIL;
		typedef s11nlite::micro_api<MapIL> MicroMap;
		MicroMap micromap;
		MapIL mil;
		mil[0] = li;
		micromap.save( mil, mfile );
		s11n::cleanup_serializable( mil );
		CERR << "^^^ Should see more Type1/2 dtor calls.\n";
		s11nlite::save( mil, std::cout );
		CERR << "^^^ should be empty.\n";

		CERR << "Structure of data file looks more or less like:\n";
		std::auto_ptr<s11nlite::node_type> nodecheck( s11nlite::load_node( mfile ) );
		s11n::debug::dump_node_structure<s11nlite::node_type>( *nodecheck );
		s11nlite::save( *nodecheck, std::cout );

		MapIL * mil2 = micromap.load( mfile );
		if( ! mil2 )
		{
			S11N_THROW( "Load of mil2 from file failed!" );
		}

		s11n::cleanup_serializable( mil2 );
		CERR << "^^^ Should see more Type1/2 dtor calls.\n";

	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
        return 0;
}
