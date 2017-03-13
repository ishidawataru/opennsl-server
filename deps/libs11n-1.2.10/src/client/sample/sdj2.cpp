////////////////////////////////////////////////////////////
// A demo of s11nlite, written for the Software Developer's
// Journal (thus the minimalism!).
// Required linker arguments: -rdynamic -ls11n
#include <s11n.net/s11n/s11nlite.hpp> // s11nlite framework
#include <s11n.net/s11n/micro_api.hpp> // minimalistic s11n API
#include <s11n.net/s11n/proxy/std/map.hpp> // a proxy for std::map
#include <s11n.net/s11n/proxy/pod/int.hpp> // ... int
#include <s11n.net/s11n/proxy/pod/string.hpp> // ... std::string

int
main( int argc, char **argv )
{
   std::string format = (argc>1) ? argv[1] : "parens";
   s11nlite::serializer_class( format ); // define output format
   try
   {
      typedef std::map<int,std::string> NumberMap;
      NumberMap map;
      map[0] = "zero";
      map[1] = "one";
      map[2] = "two";

      std::string myfile = "mymap.s11n";

      // One of several possible ways to save an object:
      s11nlite::micro_api<NumberMap> micro;
      micro.save( map, myfile );

      // One of several possible ways to load it:
      s11n::cleanup_ptr<NumberMap> delist( micro.load( myfile ) );

      // Now let's take a look at the loaded copy:
      s11nlite::save( *delist, std::cout );

      // The deserialized copy is cleaned up by
      // cleanup_ptr when we leave this scope.
   }
   catch( const std::exception & ex )
   {
      std::cerr << "EXCEPTION: " << ex.what() << "\n";
      return 1;
   }
   return 0;
}
