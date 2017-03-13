#ifndef SerializableBase_HPP
#define SerializableBase_HPP

#include <s11n.net/s11n/s11nlite.hpp>
// ^^^ Deps can be cut down significant by including, e.g. only
// s11n_node.hpp here. The catch is that s11nlite does not strictly
// define it's node_type to be s11n_node, so for pedantic correctness
// reasons include s11nlite.hpp here. There are ways around such a big
// dependency here (which costs clients compile times), though.

class SerializableBase
{
protected:
	SerializableBase() : common(4) {}
public:
	virtual ~SerializableBase() {}
	int common;
	virtual int go() = 0;
	virtual bool operator()( s11nlite::node_type & dest ) const;
	virtual bool operator()( const s11nlite::node_type & src );
};


#endif // SerializableBase_HPP
