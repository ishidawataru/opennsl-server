////////////////////////////////////////////////////////////
// A demo of s11n showing one way to save shared/static
// objects.
// Required linker arguments: -rdynamic -ls11n
#include <memory> // auto_ptr
#include <s11n.net/s11n/s11nlite.hpp> // s11nlite framework

/************************************************************************
  If you want to see how making types Serializable affects compilation
  speed, try compiling this file with USE_LIGHTWEIGHT_S11N set to 0
  and then once more with it set to 1.  On my box, 1.4GHz: 1 == 1.7s,
  0 == 3.4s. Aside from demonstrating the direct compile-time costs of
  s11n, it also demonstrates how we can use compile-time information
  to choose between different serialization strategies. Similar tricks
  can be achieved via template metaprogramming techniques.

  "Lightweight" serialization basically meaning it produces
  less output and uses only techniques which do not require s11n
  registrations of participating types. This approach won't work in
  all use cases, but can be used in most cases involving only PODs or
  non-nested containers of PODs.
************************************************************************/
#define USE_LIGHTWEIGHT_S11N 1
#if USE_LIGHTWEIGHT_S11N
#  include <s11n.net/s11n/proxy/mapish.hpp> // generic map-related de/ser algos
#  include <s11n.net/s11n/proxy/listish.hpp> // generic list-related de/ser algos
#else
#  include <s11n.net/s11n/proxy/std/list.hpp> // install list proxy
#  include <s11n.net/s11n/proxy/std/map.hpp> // install map proxy
#  include <s11n.net/s11n/proxy/pod/int.hpp> // install int proxy
#  include <s11n.net/s11n/proxy/pod/string.hpp> // install std::string proxy
#endif /* USE_LIGHTWEIGHT_S11N */


// Some commonly-used types we want to serialize.
// For our example, assume we will use these to hold
// app-wide shared data.
typedef std::map<std::string,std::string> MapType;
typedef std::list<std::string> ListType;

/** Returns a shared instance of SharedT. All calls to this function
    return the same object instance. Behaviour post-main() is
    undefined.
*/
template <typename SharedT>
SharedT & shared()
{
	static SharedT bob;
	return bob;
}


/************************************************************************
SharedData_s11n is a Serializable which de/serializes some shared<T>()
objects.

If you are a template metaprograming wizard, you could theoretically use
typelists to define which shared objects should be serialized.
*/
struct SharedData_s11n
{

	/** Serialize operator. */
	template <typename NodeType>
	bool operator()( NodeType & dest ) const
	{
		typedef ::s11n::node_traits<NodeType> TR;
		TR::class_name( dest, "SharedData" );
		MapType & map = shared<MapType>();
		ListType & list = shared<ListType>();
#if USE_LIGHTWEIGHT_S11N
		return s11n::map::serialize_streamable_map( dest, "map", map )
			&&
			s11n::list::serialize_streamable_list( dest, "list", list );
#else
		return s11n::serialize_subnode( dest, "map", map )
			&&
			s11n::serialize_subnode( dest, "list", list );
#endif
	}

	/** Deserialize operator. */
	template <typename NodeType>
	bool operator()( const NodeType & src ) const
	{
		MapType & map = shared<MapType>();
		ListType & list = shared<ListType>();
		map.clear();
		list.clear();
#if USE_LIGHTWEIGHT_S11N
		return s11n::map::deserialize_streamable_map( src, "map", map )
			&&
			s11n::list::deserialize_streamable_list( src, "list", list );
#else
		return s11n::deserialize_subnode( src, "map", map )
			&&
			s11n::deserialize_subnode( src, "list", list );
#endif
	}

};

/************************************************************************
Note that we never register SharedData_s11n as a Serializable. The
reasons we can get away with that here are:

a) We won't be deserializing pointers to that type, which means we won't
be invoking a classloader.

b) That type already implements the "default" Serializable interface.

c) We aren't nesting this type anywhere, and therefor know that nobody
else needs info like its stringified class name (which is only neessary
for (a)).

************************************************************************/


int
main( int argc, char **argv )
{
   // optional argv[1]: Serializer class name for s11nlite.
   std::string format = (argc>1) ? argv[1] : "parens";
   s11nlite::serializer_class( format ); // define output format
   try
   {
	   std::string myfile = "shared_objects.s11n";

	   { // populate shared objects:
		   MapType & map = shared<MapType>();
		   ListType & list = shared<ListType>();
		   map["foo"] = "bar";
		   map["barre"] = "yet another property";
		   list.push_back( "hi" );
		   list.push_back( "world" );
	   }

	   // Save shared data:
	   SharedData_s11n proxy;
	   if( ! s11nlite::save( proxy, myfile ) )
	   {
		   throw s11n::s11n_exception( "Saving object to %s failed.", myfile.c_str() );
	   }
	   
	   // Load shared data:
	   std::auto_ptr<s11nlite::node_type> node( s11nlite::load_node( myfile ) );
	   if( ! node.get() )
	   {
		   throw s11n::s11n_exception( "Loading node from %s failed.", myfile.c_str() );
	   }
	   if( ! s11n::deserialize( *node, proxy ) )
	   {
		   throw s11n::s11n_exception( "Deserializing object from %s failed.", myfile.c_str() );
		   /**
		      As a general rule, if deserialization fails, the
		      internal state of the deserializing object is
		      undefined. Some algorithms offer specific
		      guarantees in the face of failure, but as a
		      general rule no specific behaviour vis-a-vis
		      object consistency after failed deserialization
		      can be guaranteed by s11n's core
		      API/conventions.
		   */
	   }

	   // Now let's take a look at the loaded copy:
	   s11nlite::save( proxy, std::cout );
   }
   catch( const std::exception & ex )
   {
      std::cerr << "EXCEPTION: " << ex.what() << "\n";
      return 1;
   }
   return 0;
}
