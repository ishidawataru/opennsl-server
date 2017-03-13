// Demonstration for loading a Serializable class from a DLL.

#include "SerializableFinal.hpp"

bool SerializableFinal::operator()( s11nlite::node_type & dest ) const
{
	typedef s11nlite::node_traits TR;
	TR::class_name( dest, "SerializableFinal" );
	s11nlite::serialize_subnode( dest, "base", base );
	return true;
}

bool SerializableFinal::operator()( const s11nlite::node_type & src )
{
	typedef s11nlite::node_traits TR;
	
	const s11nlite::node_type * ch = s11n::find_child_by_name( src, "base" );
	if( ! ch )
	{
		CERR << "Deser of 'base' member failed: node not found!\n";
		return false;
	}
	base = s11nlite::deserialize<SerializableBase>( *ch );
	return 0 != base;
}


#define S11N_TYPE SerializableFinal
#define S11N_TYPE_NAME "SerializableFinal"
#include <s11n.net/s11n/reg_s11n_traits.hpp>
