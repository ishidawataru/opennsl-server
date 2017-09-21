// Demonstration for loading a Serializable class from a DLL.

#include "SerializableTwo.hpp"


int SerializableTwo::go() { return 2; }

bool SerializableTwo::operator()( s11nlite::node_type & dest ) const
	{
		this->SerializableBase::operator()( dest );
		typedef s11nlite::node_traits TR;
		TR::class_name( dest, "SerializableTwo" );
		return true;
	}

bool SerializableTwo::operator()( const s11nlite::node_type & src )
{
	this->SerializableBase::operator()( src );
	typedef s11nlite::node_traits TR;
	return true;
}

#define S11N_TYPE SerializableTwo
#define S11N_TYPE_NAME "SerializableTwo"
#define S11N_BASE_TYPE SerializableBase
#include <s11n.net/s11n/reg_s11n_traits.hpp>
