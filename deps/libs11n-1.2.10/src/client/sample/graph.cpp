////////////////////////////////////////////////////////////////////////
// Demonstration/test file for libs11n serializing a graph.
// It's not pretty, and i'm sure someone more proficient with
// graphs could do a MUCH nicer job.
// ----- stephan@s11n.net

#include <sstream>
#include <iostream>
#include <memory>
#include <vector>
#include <utility> // make_pair()

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/micro_api.hpp>
#include <s11n.net/s11n/algo.hpp>
#include <s11n.net/s11n/type_traits.hpp>
#include <s11n.net/s11n/s11n_debuggering_macros.hpp>


// Register some proxies we will need:
#include <s11n.net/s11n/proxy/std/map.hpp>
#include <s11n.net/s11n/proxy/std/vector.hpp>
#include <s11n.net/s11n/proxy/pod/size_t.hpp>
#include <s11n.net/s11n/proxy/pod/double.hpp>

/**
   graph_node is demonstration class for serializing a linked-list
   structure. Note that it is not directly a Serializable.

   The de/serialization is untested with a pointer-qualified T, but in
   theory it should work.
*/
template <typename T>
class graph_node
{
public:
	typedef T data_type;
	typedef graph_node<T> link_type;

	~graph_node()
	{
		CERR << "~graph_node() @ " << std::hex << this << "\n";
	}

	data_type data;
	link_type * left;
	link_type * right;
	graph_node() : data(), left(0), right(0) {}
	// Curious that ^^^^ C++ lets us use that to init a pointer to 0,
	// whereas this code generates a warning: int * i();
	// "the address of 'int *i()' will always be true'

	graph_node( data_type d, link_type * l = 0, link_type * r = 0 )
		: data(d), left(l), right(r) {}

};

/**
   A helper type to walk a graph_node<T> graph
   and assign unique identifiers to each node.
*/
template <typename T>
class graph_node_walker
{
public:
	typedef graph_node<T> graph_node_type;
	typedef typename graph_node_type::data_type data_type;
	typedef size_t id_type;
	typedef std::vector<const graph_node_type *> list_type;
	typedef std::map<id_type,const graph_node_type *> map_type;

private:
	typedef std::map<const graph_node_type *,id_type> visits_map;
	map_type m_map; // maps IDs to graph_nodes
	visits_map m_visits; // remembers who we've already visited
	id_type m_id; // an incremental counter
public:
	graph_node_walker()
		: m_map(),m_visits(),m_id(0)
	{}


	/**
	   Returns the map built up by walk().
	*/
	const map_type & map() const
	{
		return this->m_map;
	}

	/**
	   Clears internal state. Call this before walk()ing different
	   nodes with this object.
	*/
	void clear()
	{
		this->m_visits.clear();
		this->m_map.clear();
	}


	/**
	   Pass this an arbitrary node and it will walk
	   all left/right links to build up the map
	   returned by map().
	*/
	bool walk( const graph_node_type * n )
	{
		if( ! n ) return false;
		id_type id = this->get_id( n );
		if( 0 == id )
		{
			id_type id = ++(this->m_id);
			this->m_visits[n] = id;
			this->m_map[id] = n;
			//CERR << "walk("<<std::hex<<n<<", val="<<n->data<<") id="<<id<<"\n";
			if( n->left ) this->walk( n->left );
			if( n->right ) this->walk( n->right );
			return true;
		}
		return false;
	};


	/**
	   Returns the id for n, or 0 if n has never been walked.
	 */
	id_type get_id( const graph_node_type * n ) const
	{
		if( ! n ) return 0;
		typename visits_map::const_iterator it = this->m_visits.find( n );
		if( this->m_visits.end() != it )
		{
			return (*it).second;
		}
		return 0;
	}

	/** Returns true if this object has walked n before. */
	bool visited( const graph_node_type * n )
	{
		return this->m_visits.end()
			!= this->m_visits.find(n);
	}
};


// Install a special proxy for pair<size_t,size_t> because we know we can
// i/ostream both components. This will significantly reduce the output
// size for that type, compared to using the full-blown size_t proxy
// defined in proxy/pod/size_t.hpp
#define S11N_TYPE std::pair<size_t,size_t>
#define S11N_TYPE_NAME "id_pair"
#define S11N_SERIALIZE_FUNCTOR ::s11n::map::streamable_pair_serializable_proxy
#include <s11n.net/s11n/reg_s11n_traits.hpp>

/**
   A rather unconventional s11n proxy to serialize a list of
   graph_node items. The implementation could be better, certainly.
*/
struct graph_node_s11n
{
        /**
	   Serialize.

	   This implementation walks src and serializes all linked items
	   in one go, rather than via recursive calls.
	*/
        template <typename NodeType, typename DataT>
        bool operator()( NodeType & dest, const graph_node<DataT> & src ) const
        {
		typedef graph_node<DataT> GNode;
		CERR << "graph_node_s11n::serialize\n";
                typedef ::s11n::node_traits<NodeType> NTR;

		typedef typename ::s11n::type_traits<DataT>::type _DataT; // stripped of any pointer part

		// First, let graph_node_walker collect the list of relationships
		// for us:
		typedef graph_node_walker<typename GNode::data_type> Walker;
		Walker gw;
		gw.walk(&src);
// 		CERR << "gw.walk("<<std::hex<<&src<<")"
// 		     << " container sizes: map=="<<std::dec<<gw.map().size()
// 		     << "\n";

		// Now we're going to do rather a lot more work than we really
		// need to...


		// Create a top-level s11n container to store the
		// graph_nodes in:
		NodeType & objs = ::s11n::create_child( dest, "nodes" );


		// We use a middle-man map to hold the inter-ID relationships:
		typedef typename Walker::id_type IdT;
		typedef std::map<IdT,std::pair<IdT,IdT> > Relations;
		Relations rmap;

		// Now walk the map provided by gw and build up our
		// internal containers...
		typedef typename Walker::map_type::const_iterator WIT;
		WIT wb = gw.map().begin();
		WIT we = gw.map().end();
		NodeType * subnode = 0; // a holder for subnodes of objs
		const GNode * at = 0; // current graph_node
		IdT atid = 0; // id of current graph_node
		for( ; we != wb; ++wb )
		{
			atid = (*wb).first;
			at = (*wb).second;
			// Map the relationships to the left/right graph_nodes:
			rmap[atid] = std::make_pair( gw.get_id( at->left ),
						     gw.get_id( at->right ) );
			// Create an s11n container to store this graph_node's data in:
			subnode = &::s11n::create_child( objs, "node" );
			// Store the ID of the graph_node:
			NTR::set( *subnode, "id", atid );
			// Serialize at->data to yet another subnode:  (don't mix unrelated properties and objects in the same node)
			if( ! ::s11n::serialize_subnode<NodeType,_DataT>( *subnode, "data", at->data ) )
			{
				throw ::s11n::s11n_exception( "%s:%d: Serialization of graph_node->data component failed.", __FILE__,__LINE__ );
			}
		}
		// Now serialize the relationships map we built up:
 		return ::s11n::serialize( ::s11n::create_child( dest, "relations" ), rmap );
        }

        // deserialize
        template <typename NodeType, typename DataT>
        bool operator()( const NodeType & src, graph_node<DataT> & dest ) const
        {
		typedef graph_node<DataT> GNode;
		typedef typename GNode::data_type GData;
                typedef ::s11n::node_traits<NodeType> TR;
		//CERR << "graph_node_s11n::deserialize node == " << std::hex << &dest << "\n";

		const NodeType * nodes = ::s11n::find_child_by_name( src, "nodes" );
		if( ! nodes )
		{
			throw s11n::s11n_exception( "%s:%d: 'nodes' subnode not found in graph_node s11n data!", __FILE__,__LINE__ );
		}

		typedef size_t IdT;
		typedef std::map<IdT,std::pair<IdT,IdT> > Relations;
		Relations rmap;
		const NodeType * rels = ::s11n::find_child_by_name( src, "relations" );
		if( ! rels )
		{
			throw s11n::s11n_exception("%s:%d: 'relations' node not found in graph_node s11n data!", __FILE__,__LINE__ );
		}
		if(  !::s11n::deserialize( *rels, rmap ) )
		{
			throw s11n::s11n_exception( "%s:%d: Failed to deserialize 'relations' node from graph_node s11n data!", __FILE__,__LINE__ );
		}

		// CERR << "rmap.size() == " << rmap.size() << "\n";
		typedef std::map<size_t,GNode *> GIdMap;
		GIdMap idmap;
		const NodeType * ch = 0;
		typedef typename TR::child_list_type::const_iterator ChIt;
		ChIt cb = TR::children(*nodes).begin();
		ChIt ce = TR::children(*nodes).end();
		size_t id = 0;
		size_t firstid = 0;
		typedef ::s11n::cleanup_ptr<GNode> ClPtr;
		ClPtr gch;
		for( ; ce != cb; ++cb )
		{
			ch = *cb;
			id = TR::get( *ch, "id", (size_t)0 );
			if( 0 == id )
			{
				throw s11n::s11n_exception("%s:%d: Missing 'id' field in graph_node s11n data!", __FILE__,__LINE__ );
			}
			if( 0 == firstid ) firstid = id;
			ch = ::s11n::find_child_by_name( *ch, "data" );
			if( ! ch )
			{
				throw s11n::s11n_exception( "%s:%d: 'data' subnode not found in graph_node s11n data!", __FILE__,__LINE__ );
			}
			if( firstid == id )
			{
				// We have to treat the "head" ID specially, and deser direct into dest. If we
				// use a temp copy and reassign later then we end up with an extra item in the graph.
				idmap[id] = &dest;
				if( ! ::s11n::deserialize<NodeType,typename ::s11n::type_traits<GData>::type>( *ch, dest.data ) )
				{
					throw s11n::s11n_exception( "%s:%d: Deserialize of graph_node->data failed!", __FILE__,__LINE__ );
				}
			}
			else
			{
				gch.take( new GNode );
				if( ! ::s11n::deserialize<NodeType,typename ::s11n::type_traits<GData>::type>( *ch, gch->data ) )
				{
					throw s11n::s11n_exception( "%s:%d: Deserialize of graph_node->data failed!", __FILE__,__LINE__ );
				}
				idmap[id] = gch.release(); // transfer ownership.
			}
			//CERR << "mapping idmap["<<id<<"] == " << std::hex << idmap[id] << "\n";
		}

		idmap[firstid] = &dest;
		// CERR << "idmap.size() == " << idmap.size() << "\n";
		typedef typename GIdMap::iterator IDIt;
		IDIt idb = idmap.begin();
		IDIt ide = idmap.end();
		GNode * thegn = 0;
		for( ; ide != idb; ++idb )
		{
			id = (*idb).first;
			thegn = (*idb).second;
			if( ! thegn )
			{
				throw s11n::s11n_exception( "%s:%d: NULL node found in id-to-graph_node map!", __FILE__,__LINE__ );
			}
			thegn->left = idmap[rmap[id].first];
			thegn->right = idmap[rmap[id].second];
// 			CERR << "linking ["<<id<<"] "<<std::hex<<thegn<<": left=" << std::hex << thegn->left
// 			     << " right="<< std::hex << thegn->right << "\n";
		}
// 		typedef graph_node_walker<typename GNode::data_type> Walker;
// 		Walker gw;
// 		gw.walk(thegn);
// 		CERR << "gw.walk("<<std::hex<<thegn<<")"
// 		     << " container sizes: map=="<<std::dec<<gw.map().size()
// 		     << "\n";

                return true;
        }

}; // graph_node_s11n


#define S11N_TEMPLATE_TYPE graph_node
#define S11N_TEMPLATE_TYPE_NAME "graph_node"
#define S11N_TEMPLATE_TYPE_PROXY graph_node_s11n
#include <s11n.net/s11n/proxy/reg_s11n_traits_template1.hpp>

namespace s11n {
	/**
	   A cleanup functor for graph_node<> types.
	*/
        template <typename ValT >
        struct default_cleanup_functor< graph_node< ValT > >
        {
                typedef graph_node< ValT > cleaned_type;

		/**
		   Walks c, cleans up all 'data' members and deletes each
		   left/right link in the list. After this call, the only
		   still-existing item in the graph is c, and it is empty.
		 */
		void operator()( cleaned_type & c )
		{
			typedef typename cleaned_type::data_type _GDT;
			typedef typename ::s11n::type_traits<_GDT>::type GDT;
			if( c.left || c.right )
			{
				CERR << "Cleaning up graph_node graph...\n";
				typedef graph_node_walker<_GDT> Walker;
				Walker gw;
				gw.walk(&c);
				typedef typename Walker::map_type::const_iterator WIT;
				WIT wb = gw.map().begin();
				WIT we = gw.map().end();
				cleaned_type * at = 0; // current graph_node
				for( ; we != wb; ++wb )
				{
					at = const_cast<cleaned_type*>( (*wb).second ); // extremely unfortunate
					if( at == &c ) continue; // don't nuke this one yet!
					::s11n::cleanup_serializable<GDT>( at->data );
					delete at;
				}
				c.left = c.right = 0;
			}
			::s11n::cleanup_serializable<GDT>( c.data );
		}
        };

} // namespace

void test_graph()
{
	typedef graph_node<double> GNode;
	GNode * n = new GNode;
	s11n::cleanup_ptr<GNode> cleaner(n);
	typedef std::vector<GNode *> GVec;
	GVec vec;
	vec.push_back( n );
	GNode * nnode = 0;
	GNode * prev = n;
	for( int i = 1; i < 5; i++ )
	{
		nnode = new GNode( (2.0*i), prev, 0 );
		prev->right = nnode;
		prev = nnode;
		vec.push_back( nnode );
	}
	nnode->right = n; // link tail back to head

	CERR << "n->data == " << std::hex << n->data << "\n";

	GVec::iterator b = vec.begin();
	GVec::iterator e = vec.end();
	for( ; e != b; ++b )
	{
		nnode = (*b);
		CERR << "Node "<<std::hex << nnode << ", value=="<<nnode->data
		     << ", left="<<std::hex<<nnode->left
		     << ", right="<<std::hex<<nnode->right
		     << "\n";
	}

	typedef s11nlite::micro_api<GNode> MicroT;
	MicroT micro;

	std::string fname = "graph.s11n";
	if( (! micro.save( *n, std::cout ))
	     ||
	    (!micro.save( *n, fname ))
	    )
	{
		throw s11n::s11n_exception( "%s:%d: Saving graph failed!", __FILE__,__LINE__ );
	}

	cleaner.clean();
	CERR << "cleaner.get() == " << std::hex << cleaner.get() << "\n";
	if( 0 != cleaner.get() )
	{
		throw s11n::s11n_exception( "%s:%d: Assertion failed: cleaner.get() should be 0 at this point.", __FILE__,__LINE__ );
	}

	CERR << "Saved file '"<<fname<<"'.\n";

	cleaner.take( micro.load( fname ) );
	CERR << "Deser'd graph_node = " << std::hex << cleaner.get() << "\n";
	if( ! cleaner.get() )
	{
		throw s11n::s11n_exception( "%s:%d: Failed to load '%s'!", __FILE__,__LINE__,fname.c_str() );
	}
	typedef graph_node_walker<GNode::data_type> Walker;
	Walker gw;
	gw.walk( cleaner.get() );
	CERR << "walker says we have this many nodes: " << gw.map().size() << "\n";
	micro.save( *cleaner, std::cout );
	micro.save( *cleaner,"graph.reserialized.s11n" );
}

int
main(int argc, char **argv)
{
 	using namespace s11n::debug;
 	trace_mask( trace_mask() | TRACE_CLEANUP ); // TRACE_INFO | TRACE_TRIVIAL );

	try
	{
		test_graph();

	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
	CERR << "Bye, bye!\n";
	return 0;
}
