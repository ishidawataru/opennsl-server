#ifndef pool_OBJECT_POOL_S11N_HPP_INCLUDED
#define pool_OBJECT_POOL_S11N_HPP_INCLUDED 1


////////////////////////////////////////////////////////////////////////
// object_pool_s11n.hpp
// Defines a serialization proxy for pool::object_pool objects.
// Should be included after object_pool.hpp and s11n[lite].hpp.
////////////////////////////////////////////////////////////////////////

#include <s11n.net/s11n/proxy/mapish.hpp>
// Client must include proxies for pool's key/object types, if needed.

namespace pool {

        struct S11N_EXPORT_API object_pool_s11n
        {

		template <typename NodeT, typename KeyT, typename ObjectT>
                bool operator()( NodeT & dest, const object_pool<ObjectT,KeyT> & src ) const
                {
                        typedef s11n::node_traits<NodeT> NTR;
                        NTR::set( dest, "auto_delete", src.auto_delete() ? 1 : 0 );
                        return s11n::map::serialize_map( dest, "objects", src.map() );
                }

		template <typename NodeT, typename KeyT, typename ObjectT>
                bool operator()( const NodeT & src, object_pool<ObjectT,KeyT> & dest ) const
                {
                        typedef s11n::node_traits<NodeT> NTR;
			dest.destroy_all();
                        dest.auto_delete( NTR::get( src, "auto_delete", dest.auto_delete() ? 1 : 0 ) );
                        bool ret = s11n::map::deserialize_map( src, "objects", dest.map() );
                        dest.rebuild_backrefs(); // we bypassed the pool API here ^^^^^ and need to fix that :/
                        return ret;
                }
        };

}// namespace
#define S11N_TEMPLATE_TYPE pool::object_pool
#define S11N_TEMPLATE_TYPE_NAME "pool::object_pool"
#define S11N_TEMPLATE_TYPE_PROXY pool::object_pool_s11n
#include <s11n.net/s11n/proxy/reg_s11n_traits_template2.hpp>

#endif // pool_OBJECT_POOL_S11N_HPP_INCLUDED
