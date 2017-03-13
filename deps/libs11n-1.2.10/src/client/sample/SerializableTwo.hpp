#ifndef my_SERIALIZABLETWO_HPP_INCLUDED
#define my_SERIALIZABLETWO_HPP_INCLUDED 1


#include "SerializableBase.hpp"

class SerializableTwo : public SerializableBase {

public:
	SerializableTwo() {}
	int go();
	virtual bool operator()( s11nlite::node_type & dest ) const;
	virtual bool operator()( const s11nlite::node_type & src );
};


#endif // my_SERIALIZABLETWO_HPP_INCLUDED
