// Demonstration of serializing a map of coordinate points to client-side objects.
// For s11n 1.1.x

// Include core s11nlite framework:
#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/micro_api.hpp> // experimental type

// Include the COUT and CERR macros (only for demonstration - clients shouldn't do this).
#include <s11n.net/s11n/s11n_debuggering_macros.hpp>

// We will be serializing a std::map. For our case we can use the default proxy:
#include <s11n.net/s11n/proxy/std/map.hpp>


// We use stringstreams to simulate files, to save us from writing temp files
#include <sstream>

#include <memory> // auto_ptr

#include "coord.hpp"
#include "coord_s11n.hpp"


/**
   A simple class for demonstrating serialization.
*/
class some_obj
{

public:
	some_obj() : m_name("unnamed"),m_role("")
	{
	}

	some_obj( const std::string & n ) : m_name(n),m_role("")
	{
	}

	std::string name() const { return this->m_name; }
	void name( const std::string & n ) { this->m_name = n; }

	std::string role() const { return this->m_role; }
	void role( const std::string & r ) { this->m_role = r; }

private:
	std::string m_name;
	std::string m_role;

};

/**
   A non-intrusive serialization proxy for some_obj.
*/
struct some_obj_s11n
{
        // serialize
        template <typename NodeType>
        bool operator()( NodeType & dest, const some_obj & src ) const
        {
                typedef ::s11n::node_traits<NodeType> TR;
                TR::class_name( dest, "some_obj" );
                TR::set( dest, "name", src.name() );
                TR::set( dest, "role", src.role() );
                return true;
        }

        // deserialize
        template <typename NodeType>
        bool operator()( const NodeType & src, some_obj & dest ) const
        {
                typedef ::s11n::node_traits<NodeType> TR;
                dest.name( TR::get( src, "name", dest.name() ) );
                dest.role( TR::get( src, "role", dest.role() ) );
                return true;
        }

}; // some_obj_s11n

/**
   Register some_obj_s11n as the serialization proxy for some_obj.
*/
#define S11N_TYPE some_obj
#define S11N_TYPE_NAME "some_obj"
#define S11N_SERIALIZE_FUNCTOR some_obj_s11n
#include <s11n.net/s11n/reg_s11n_traits.hpp>

typedef std::map<coord,some_obj> ObjMap; // convenience typedef

////////////////////////////////////////////////////////////////////////
// The following registration of ObjMap is optional, and only necessary
// if:
// a) We want to change the default class name used by the proxy.
// Or:
// b) We want to install our own proxy functor, rather than use the
//    default functor.
//
// #define S11N_TYPE ObjMap
// #define S11N_TYPE_NAME "ObjMap"
// #define S11N_SERIALIZE_FUNCTOR ::s11n::map::map_serializable_proxy
// #include <s11n.net/s11n/reg_s11n_traits.hpp>
////////////////////////////////////////////////////////////////////////


ObjMap & obj_map()
{
	static ObjMap bob;
	return bob;
}

void setup_map()
{
	ObjMap & m = obj_map();
	m[coord(1,4)] = some_obj("Fred");
	m[coord(1,4)].role( "Husband of Wilma" );
	m[coord(20,7)] = some_obj("Barney");
	m[coord(20,7)].role( "Fred's best friend." );
	m[coord(-10,-13)] = some_obj("Wilma");
	m[coord(-10,-13)].role( "The lovely wife." );
	m[coord(-12,-17)] = some_obj("Betty");
	m[coord(-12,-17)].role( "The best friend's lovely wife." );
}


int main( int argc, char ** argv )
{

	COUT << "s11n demo app.\n";
	setup_map();
	try
	{
		s11nlite::serializer_class( "parens" );


		COUT << "We've just set up our map:\n";

		std::ostringstream obuf;
		if( ! s11nlite::save( obj_map(), obuf ) )
		{
			throw s11n::s11n_exception( "%s:%d: save( obj_map(), buffer ) failed!", __FILE__,__LINE__ );
		}

		COUT << obuf.str();

		COUT << "Re-deserializing it to another object...\n";
		ObjMap map2;
		if( ! s11nlite::s11n_cast( obj_map(), map2 ) )
		{
			throw s11n::s11n_exception( "%s:%d: s11n_cast() failed!", __FILE__,__LINE__ );
		}

		// Now create a different serializer to output it with.
		// We'll do this the hard way, for demonstration purposes:
		std::string outfmt = "simplexml";
		typedef std::auto_ptr<s11nlite::serializer_interface> SAP;
		SAP ser( s11nlite::create_serializer( outfmt ) );
		if( ! ser.get() )
		{
			throw s11n::s11n_exception( "%s:%d: Serializer class not found: %s", __FILE__,__LINE__,outfmt.c_str() );
		}
		s11nlite::node_type node;
		if( ! s11nlite::serialize( node, map2 ) )
		{
			throw s11n::s11n_exception( "%s:%d: serialize(node,map2) failed!", __FILE__,__LINE__ );
		}
		COUT << "New copy looks like:\n";
		ser->serialize( node, std::cout );
		// It would be much easier to do:
		// s11nlite::serializer_class( outfmt );
		// s11nlite::save( map2, std::cout );

		COUT << "And node from s11nlite:\n";
		s11nlite::save( node, std::cout );
		COUT << "And map2 from s11nlite:\n";
		s11nlite::save( map2, std::cout );

		s11nlite::micro_api<ObjMap> micro;

		std::string ofile = "demo_coord.s11n";
		if( ! micro.save( obj_map(), ofile ) )
		{
			throw s11n::s11n_exception( "%s:%d: micro.save(%s) failed!", __FILE__, __LINE__,ofile.c_str() );
		} else
		{
			COUT << "Saved using micro_api ==> " << ofile << "\n";
		}

		std::auto_ptr<ObjMap> fromfile( micro.load( ofile ) );
		if( ! fromfile.get() )
		{
			throw s11n::s11n_exception("%s:%d: micro.load(%s) failed!", __FILE__,__LINE__,ofile.c_str());
		}
		else
		{
			COUT << "ObjMap loaded using micro.load("<<ofile<<"):\n";
			micro.save( *fromfile, std::cout );
		}
	}
	catch( const ::s11n::s11n_exception & sex )
	{
		CERR << "Exception during deserialization: " << sex.what() << "\n";
		return 1;
	}



	return 0;
}

