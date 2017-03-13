#ifndef demo_COORD_S11N_HPP_INCLUDED
#define demo_COORD_S11N_HPP_INCLUDED 1
// include this header after including both coord.hpp and the basic s11n-related headers,
// e.g. s11nlite.hpp

/**
   A non-intrusive serialization proxy for the coord class.
*/
struct coord_s11n
{
        // serialize
        template <typename NodeType>
        bool operator()( NodeType & dest, const coord & src ) const
        {
                typedef ::s11n::node_traits<NodeType> TR;
                TR::class_name( dest, "coord" );
                TR::set( dest, "x", src.x() );
                TR::set( dest, "y", src.y() );
                return true;
        }

        // deserialize
        template <typename NodeType>
        bool operator()( const NodeType & src, coord & dest ) const
        {
                typedef ::s11n::node_traits<NodeType> TR;
                dest.x( TR::get( src, "x", dest.x() ) );
                dest.y( TR::get( src, "y", dest.y() ) );
                return true;
        }

}; // coord_s11n

// Now register coord_s11n as the proxy with the s11n framework:
#define S11N_TYPE coord
#define S11N_TYPE_NAME "coord"
#define S11N_SERIALIZE_FUNCTOR coord_s11n
#include <s11n.net/s11n/reg_s11n_traits.hpp>

#endif // demo_COORD_S11N_HPP_INCLUDED

