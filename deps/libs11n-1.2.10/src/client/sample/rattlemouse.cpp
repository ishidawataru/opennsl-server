////////////////////////////////////////////////////////////////////////
// Test/demo code contributed by Rattlemouse <rattlemouse@apeha.ru>.
////////////////////////////////////////////////////////////////////////
#include "stdlib.h"
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include <list>
using namespace std;
#include <memory>
#include <s11n.net/s11n/s11nlite.hpp> // s11n & s11nlite frameworks
#include <s11n.net/s11n/proxy/std/list.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>
#include <s11n.net/s11n/proxy/std/set.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>
#include <s11n.net/s11n/proxy/pod/double.hpp>

typedef s11nlite::node_type CacheNode;

#define SerializationInit(A, ClassName) typedef s11n::node_traits<s11nlite::node_type> _NT; \
    _NT::class_name(A, ClassName);
#define DSerializationInit() typedef s11n::node_traits<s11nlite::node_type> _NT;
    	
#define SerializeClass(node, name, pntr) s11nlite::serialize_subnode(node, name, pntr);
#define SerializeList(node, name, list1) s11n::list::serialize_list(node, name, list1);
#define SerializeMap(node, name, map1) s11n::map::serialize_map(node, name, map1);
#define SerializeString(node, name, str) _NT::set(node, name, str);
#define SerializeInt(node, name, str) _NT::set<int>(node, name, str);
#define SerializeChar(node, name, str) _NT::set<char>(node, name, str);
#define SerializeBool(node, name, str) _NT::set<bool>(node, name, str);
#define SerializeFloat(node, name, str) _NT::set<float>(node, name, str);
#define SerializeDouble(node, name, str) _NT::set<double>(node, name, str);
#define SerializeUlong(node, name, str) _NT::set<ulong>(node, name, str);
#define SerializeLong(node, name, str) _NT::set<long>(node, name, str);
#define SerializeTime(node, name, str) _NT::set<time_t>(node, name, str);

#define DSerializeClass(node, name, pntr) s11nlite::deserialize_subnode(node, name, pntr);
#define DSerializeList(node, name, list1) s11n::list::deserialize_list(node, name, list1);
#define DSerializeMap(node, name, map1) s11n::map::deserialize_map(node, name, map1);
#define DSerializeString(node, name, str, dv) str = _NT::get<std::string>(node, name, dv);
#define DSerializeInt(node, name, str, dv) str = _NT::get<int>(node, name, dv);
#define DSerializeChar(node, name, str, dv) str = _NT::get<char>(node, name, dv);
#define DSerializeBool(node, name, str, dv) str = _NT::get<bool>(node, name, dv);
#define DSerializeFloat(node, name, str, dv) str = _NT::get<float>(node, name, dv);
#define DSerializeDouble(node, name, str, dv) str = _NT::get<double>(node, name, dv);
#define DSerializeUlong(node, name, str, dv) str = _NT::get<ulong>(node, name, dv);
#define DSerializeLong(node, name, str, dv) str = _NT::get<long>(node, name, dv);
#define DSerializeTime(node, name, str, dv) str = _NT::get<time_t>(node, name, dv);

#define SChar(A) SerializeChar(src, #A, A)
#define SCharIF(A) if(A != 0) SerializeChar(src, #A, A)
#define DChar(A) DSerializeChar(dest, #A, A, '\0')
#define SStr(A) SerializeString(src, #A, A)
#define SStrIF(A) if(!A.empty()) SerializeString(src, #A, A)
#define DStr(A) DSerializeString(dest, #A, A, std::string())
#define SBool(A) SerializeInt(src, #A, A)
#define SBoolIF(A) if(A) SerializeInt(src, #A, A)
#define DBool(A) DSerializeInt(dest, #A, A, false)
#define SInt(A) SerializeInt(src, #A, A)
#define SIntIF(A) if(A != 0) SerializeInt(src, #A, A)
#define DInt(A) DSerializeInt(dest, #A, A, 0)
#define SFloat(A) SerializeFloat(src, #A, A)
#define SFloatIF(A) if(A != 0.0f) SerializeFloat(src, #A, A)
#define DFloat(A) DSerializeFloat(dest, #A, A, 0.0f)
#define STime(A) SerializeTime(src, #A, A)
#define DTime(A) DSerializeTime(dest, #A, A, 0)
#define SULong(A) SerializeUlong(src, #A, A)
#define SULongIF(A) if(A != 0) SerializeUlong(src, #A, A)
#define DULong(A) DSerializeUlong(dest, #A, A, 0)
#define SLong(A) SerializeLong(src, #A, A)
#define SLongIF(A) if(A != 0) SerializeLong(src, #A, A)
#define DLong(A) DSerializeLong(dest, #A, A, 0)


class Cl1
{
public:
	int i;
	string test;
	set<int> lst;
	bool operator()( CacheNode & src ) const
{
	SerializationInit(src, "Cl1");
	SInt(i);
	SStr(test);
		//SerializeList(src, "lst", lst)
	SerializeList(src, "lst", lst)
	return true;
}

bool operator()( const CacheNode & dest )
{
	DSerializationInit()
	DInt(i)
	DStr(test)
	DSerializeList(dest, "lst", lst)
	return true;
}
};

#define S11N_TYPE Cl1
#define S11N_TYPE_NAME "Cl1"
#include <s11n.net/s11n/reg_s11n_traits.hpp>

int main(int argc, char *argv[])
{
// 	using namespace s11n::debug;
// 	trace_mask( trace_mask() | TRACE_CTOR | TRACE_DTOR );
// 	s11nlite::serializer_class("parens");
	cout << "Building..." << endl;
	Cl1 *cl = new Cl1();
	for(long i = 0; i < 100; i++)
		cl->lst.insert(i); // push_back(i);
	cl->i = 5;
	for(long i = 0; i < 650; i++)
		cl->test += "Z";
	cout << "Serializing..." << endl;
	if(!s11nlite::save(cl, "test.s11n"))
	{
		cout << "Error" << endl;
		exit(-1);
	}
	cout << "Deleting..." << endl;
	delete(cl);
	int lc = 0;
	while(lc < 5)
	{
		++lc;
		cout << "Loading..." << endl;
		std::auto_ptr<s11nlite::node_type> node( s11nlite::load_node("test.s11n") );
		if( ! node.get() ) 
		{
	 		cout << "Error Des" << endl;
			exit(-1);
	 	}
		cl = new Cl1();
	 	cout << "Loaded Deser..." << endl;
		s11nlite::deserialize(*node, *cl );
		cout << "cl->i == " << cl->i << ", Deleting..." << endl;
		node.reset();
		cout << "Save..." << endl;
		s11nlite::save(cl, "test.s11n");
		cout << "Deleting..." << endl;
		delete(cl);
		cout << "Done..." << endl;
	}
}
