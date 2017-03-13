////////////////////////////////////////////////////////////////////////
// A demonstration application for several aspects of s11n:
//
// a) "Intrusive" serialization, in the form of member functions in
//    a base hierarchy.
//
// b) Cooperation between different Serializable hierarchies, in the form
//    of non-related Serializable types which can serialize each other.
//  
// c) The use of DLLs. Several SerializableXXX classes come with this
//    example, a couple of which we will load dynamically via DLLs.
//
// Many thanks to Mike Radford for the initial code.
////////////////////////////////////////////////////////////////////////
#include <sstream>
#include <iostream>

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/s11n_debuggering_macros.hpp>
#include <s11n.net/s11n/plugin/plugin.hpp>
#include <s11n.net/s11n/proxy/path_finder_s11n.hpp>

#include "SerializableBase.hpp"
#include "SerializableFinal.hpp"

int
main(int argc, char **argv)
{
// 	namespace DBG = ::s11n::debug;
// 	DBG::trace_mask( DBG::TRACE_FACTORY );

	/**************************************************************
	Here we demonstrate the de/serialization of multiple
	hierarchies of Serializables. Our classes are:

	- SerializableBase is an abstract type defining a virtual
	serialization interface (two operators).

	- SerializableOne and SerializableTwo publically subclass
	SerializableBase, re-implementing the serialization operators.

	- SerializableFinal is a separate Serializable type, unrelated
	to the others but with the same serialization interface.

	This example is intended to be compiled such that:

	This file links with SerializableFinal.o and
	SerializableBase.o, and Serializable{One,Two} are built as
	DLLs named Serializable{One,Two}.so, and are in
	s11n::plugin::path() (tip: try putting them in the same dir as
	this program).

	This app must be linked with the -rdynamic (or equivalent)
	linker flag, or classloading will not work. (Don't ask me for
	the details - i don't fully understand them.)
	**************************************************************/


	try
	{

		CERR << "FYI: Plugins search path =\n";
		s11nlite::save( s11n::plugin::path(), std::cout );
		// ^^^ How'd we do that? We simply made path_finder a Serializable.

#define DO_DIRECT_DLL_OPEN 0
#if DO_DIRECT_DLL_OPEN
		// The "direct" way to load a class, whether from a DLL or not:
		const std::string subtype = "SerializableTwo";
 		SerializableBase * b = s11n::cl::classload<SerializableBase>( subtype );
 		if( ! b )
 		{
 			CERR << "Failed loading class '"<<subtype<<"'\n";
 			return 1;
 		}
 		CERR << "b == " << std::hex << b << "  :)\n";
		if( b ) s11nlite::save( *b, std::cout );
 		delete b;
 		b = 0;
		//return 0;
#endif // DO_DIRECT_DLL_OPEN


		/**************************************************************
		 Now we will demonstrate how s11n can load classes
		 from DLLs without client-side code having to know
		 it's using this feature.  The feature has a few
		 limitations, but clients can overcome most of them
		 with little effort.
		**************************************************************/

		std::ostringstream os;

		// The following is a serialized SerializableFinal object:
		os << "(s11n::parens)\n"
		   << "s11n_node=(SerializableFinal \n"
		   << "base=(SerializableOne (common 4) (num 1))" // SerializableOne should be built as SerializableOne.{so,dll}
		   << ")\n";

		// Simulate a file, reading the serialized data:
		std::istringstream is( os.str() );

		// Loading the object is a one-liner:
		SerializableFinal * f = s11nlite::load_serializable<SerializableFinal>( is );
		if( ! f )
		{
			throw s11n::s11n_exception( "%s:%d: Deser of SerializableFinal failed!", __FILE__, __LINE__ );
		}
		s11nlite::serializer_class( "s11n::io::funxml_serializer" ); // set our preferred output format, if desired.
		CERR << "Our first deserialized-then-reserialized object looks like:\n";

		s11nlite::save( *f, std::cout );
		delete f;

		// Another serialized SerializableFinal object:
		os.str("");
		os << "#SerialTree 1\n"
		   << "s11n_node class=SerializableFinal {\n"
		   << "base class=SerializableTwo {\n" // SerializableTwo should be built as SerializableTwo.so
		   << "common 13\n"
		   << "}\n}\n";

		std::istringstream is2(os.str()); // using is.str(os.str()) isn't working for me???
		f = s11nlite::load_serializable<SerializableFinal>( is2 );
		if( ! f )
		{
			throw s11n::s11n_exception( "%s:%d: Deser of SerializableFinal failed!", __FILE__, __LINE__ );
		}

		CERR << "Our second deserialized-then-reserialized object looks like:\n";
		s11nlite::serializer_class( "s11n::io::funtxt_serializer" );
		s11nlite::save<SerializableFinal>( *f, std::cout );
		/**************************************************************
		^^^^^ Note that the explicit <SerializableFinal> is
		not strictly necessary here, because f is declared as a real
		SerializableFinal object (not a subtype). It is good
		practice to be explicit, however, because there are
		cases involving subtypes of Serializables where
		implicite typing won't provide the expected
		result. See the library manual for full details.
		**************************************************************/

		delete f;

	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}


	return 0;
}
