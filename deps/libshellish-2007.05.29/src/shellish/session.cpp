
#include <s11n.net/shellish/session.hpp>
#include <s11n.net/shellish/shellish_config.hpp>
#if !shellish_ENABLE_S11N
// #warning "You are building without s11n support. load/save_session() will THROW if called."
////////////////////////////////////////////////////////////////////////
// disable load/save support...
#include <stdexcept>
namespace shellish {
        bool save_session( const std::string & filename, bool includeEnv )
        {
		throw std::runtime_error( "shellish::save_session() is not supported because shellish was built without s11n support." );
        }

        bool load_session( const std::string & filename )
        {
		throw std::runtime_error( "shellish::load_session() is not supported because shellish was built without s11n support." );
	}
} // namespace
#else
////////////////////////////////////////////////////////////////////////
// enable load/save support...

#include <s11n.net/shellish/shellish.hpp> // shellish:: funcs

#include <s11n.net/s11n/s11nlite.hpp> // de/serialization framework

// s11n proxies we will need for de/ser:
#include <s11n.net/s11n/proxy/std/map.hpp>
#include <s11n.net/s11n/proxy/std/list.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>

// optional readline_cpp support
#if shellish_HAVE_LIBREADLINECPP
#    include <s11n.net/readline/Readline.hpp>
#    include <s11n.net/readline/Readline_s11n.hpp>
#endif

namespace shellish {

#if shellish_HAVE_LIBREADLINECPP
readlinecpp::Readline & readline(); // internal func defined in shellish.cpp :/
#endif

        bool save_session( const std::string & filename, bool includeEnv )
        {
                //s11nlite::node_type & node = session();
		s11nlite::node_type node;
                node.clear();
                s11nlite::serialize_subnode( node, "aliases", aliases().map() );
		if( includeEnv )
		{
			int err = env().get<int>( "?", 0 );
			env().unset( "?" ); // kludge to avoid a non-alphanumeric key in (potentially) XML data
			s11n::map::serialize_streamable_map( node, "environment", env().get_map() );
			env().set<int>( "?", err );
		}
#if shellish_HAVE_LIBREADLINECPP
                s11n::serialize_subnode( node, "readline", readline() );
#endif
                return s11nlite::save( node, filename );
        }

        bool load_session( const std::string & filename )
        {
		typedef std::auto_ptr<s11nlite::node_type> NAP;
		NAP node = NAP( s11nlite::load_node( filename ) );
                if( ! node.get() ) return false;
//                 session() = *node;
//                 delete( node );

                s11nlite::deserialize_subnode( *node.get(), "aliases", aliases().map() );
		const s11nlite::node_type * ch = 0;
		if( (ch = s11nlite::find_child( *node.get(), "environment" )) )
		{
			s11n::map::deserialize_streamable_map( *ch, env().get_map() );
		}
#if shellish_HAVE_LIBREADLINECPP
                if( (ch = s11nlite::find_child( *node.get(), "readline" )) )
                {
                        s11nlite::deserialize( *ch, readline() );
                }
#endif // shellish_HAVE_LIBREADLINECPP
                return true;
        }


} // namespace

#endif // shellish_ENABLE_S11N
