// Demonstration for loading a Serializable class from a DLL.

#include "SerializableBase.hpp"


bool SerializableBase::operator()( s11nlite::node_type & dest ) const
{
	typedef s11nlite::node_traits NT;
	NT::class_name( dest, "SerializableBase" );
	NT::set( dest, "common", common );
	return true;
}

bool SerializableBase::operator()( const s11nlite::node_type & src )
{
	typedef s11nlite::node_traits TR;
	common = TR::get( src, "common", int(0) );
	return true;
}


#define S11N_TYPE SerializableBase
#define S11N_TYPE_NAME "SerializableBase"
#define S11N_ABSTRACT_BASE
#include <s11n.net/s11n/reg_s11n_traits.hpp>
