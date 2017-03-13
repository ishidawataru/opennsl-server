#ifndef my_SERIALIZABLEONE_HPP_INCLUDED
#define my_SERIALIZABLEONE_HPP_INCLUDED 1


#include "SerializableBase.hpp"

class SerializableOne : public SerializableBase {

protected:
        int num;
public:
	SerializableOne();
	int go();
	virtual bool operator()( s11nlite::node_type & dest ) const;
	virtual bool operator()( const s11nlite::node_type & src );
};


#endif // my_SERIALIZABLEONE_HPP_INCLUDED
