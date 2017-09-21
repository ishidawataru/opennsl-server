#ifndef my_SERIALIZABLEFINAL_HPP_INCLUDED
#define my_SERIALIZABLEFINAL_HPP_INCLUDED 1

#include "SerializableBase.hpp"

class SerializableFinal {
public:
	SerializableBase * base;
	SerializableFinal(SerializableBase *_base) : base(_base) {}
	SerializableFinal() {}
	virtual ~SerializableFinal() {}
	virtual bool operator()( s11nlite::node_type & dest ) const;

	virtual bool operator()( const s11nlite::node_type & src );
};


#endif // my_SERIALIZABLEFINAL_HPP_INCLUDED
