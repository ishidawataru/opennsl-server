#ifndef S11N_NET_READLINE_S11N_HPP_INCLUDED
#define S11N_NET_READLINE_S11N_HPP_INCLUDED 1

////////////////////////////////////////////////////////////////////////
// Readline_s11n.hpp: implements an s11n proxy for readline::Readline.
// Include this header if you would like to use s11n::de/serialize()
// on a Readline object.
//
// For s11n 1.1+.
//
// License: same as that of Readline.hpp and Readline.cpp. Please
// see those files for details.
//
// Author: stephan@s11n.net
////////////////////////////////////////////////////////////////////////


#include "Readline.hpp"
#include "Readline_config.hpp"


#include "Readline_config.hpp"


#if LIBREADLINECPP_USE_LIBEDITLINE
#    include <editline/editline.h>
#    define USE_EDIT 1
#else
#  if LIBREADLINECPP_USE_LIBREADLINE
#    define USE_EDIT 1
#    include <readline/readline.h>
#    include <readline/history.h>
#  endif
#endif

#if (0 == LIBREADLINECPP_USE_LIBREADLINE) && (0==LIBREADLINECPP_USE_LIBEDITLINE)
#  define USE_EDIT 0
// maintenance note: USE_EDIT blocks only work
// for APIs which are compatible between GNU readline
// and libeditline.
#endif

#include <s11n.net/s11n/proxy/std/list.hpp> // list-related de/ser stuff
#include <s11n.net/s11n/proxy/pod/string.hpp> // list-related de/ser stuff

namespace readlinecpp {

        /**
           Readline_s11n is a Readline serialization proxy for use
           with libs11n version 0.9.x.
        */
        struct Readline_s11n
        {
                /** Serialize operator. */
                template <typename NodeType>
                bool operator()( NodeType & tgt, const Readline & src )  const
                {
                        typedef s11n::node_traits<NodeType> TR;
                        s11n::list::serialize_list( tgt, src.history() );
                        TR::class_name( tgt, "readlinecpp::Readline" );
                        return true;
                }
  
                /** Deserialize operator. */
                template <typename NodeType>
                bool operator()( const NodeType & src, Readline & tgt ) const
                {
                        // normally this history() is a one-liner, but we need to feed
                        // the results back into libreadline's history. :/
                        tgt.clear_history();
                        Readline::history_list hist;
                        s11n::list::deserialize_list( src, tgt.history() );
#if USE_EDIT
                        Readline::history_list::const_iterator it = tgt.history().begin(),
                                et = tgt.history().end();
                        for( ; et != it; ++it )
                        {
                                ::add_history( (*it).c_str() );
                        }
#endif // USE_EDIT
                        return true;
                }
        };
        
} // namespace readlinecpp

////////////////////////////////////////////////////////////////////////
// register with s11n ...
#define S11N_TYPE readlinecpp::Readline
#define S11N_TYPE_NAME "readlinecpp::Readline"
#define S11N_SERIALIZE_FUNCTOR readlinecpp::Readline_s11n
#include <s11n.net/s11n/reg_s11n_traits.hpp>
////////////////////////////////////////////////////////////////////////


#undef USE_EDIT

#endif // S11N_NET_READLINE_S11N_HPP_INCLUDED

