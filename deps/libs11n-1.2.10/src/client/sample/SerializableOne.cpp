// Demonstration for loading a Serializable class from a DLL.

#include "SerializableOne.hpp"

SerializableOne::SerializableOne()
{
}

int SerializableOne::go()
{
	return this->num;
}

bool SerializableOne::operator()( s11nlite::node_type & dest ) const
{
	if (! this->SerializableBase::operator()( dest ) ) return false ;
	typedef s11nlite::node_traits TR;
	TR::class_name( dest, "SerializableOne" );
		TR::set( dest, "num", num );
		return true;
}

bool SerializableOne::operator()( const s11nlite::node_type & src )
{
	this->SerializableBase::operator()( src );
	typedef s11nlite::node_traits TR;
	num = TR::get( src, "num", int(0) );
	return true;
}

#define S11N_TYPE SerializableOne
#define S11N_TYPE_NAME "SerializableOne"
#define S11N_BASE_TYPE SerializableBase
#include <s11n.net/s11n/reg_s11n_traits.hpp>

