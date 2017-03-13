#ifndef shellish_BUILTINS_HPP_INCLUDED
#define shellish_BUILTINS_HPP_INCLUDED 1

#include "shellish.hpp"

namespace shellish {

#define HANDLER(Func) int Func( const shellish::arguments & )

        HANDLER( shellish_debug_toggle );
        HANDLER( handle_alias );
        HANDLER( handle_chdir );
        HANDLER( handle_chdir );
        HANDLER( handle_dlload );
        HANDLER( handle_echo );
        HANDLER( handle_shellish_info );
        HANDLER( handle_env );
        HANDLER( handle_fork );
        HANDLER( handle_help );
        HANDLER( handle_help );
        HANDLER( handle_history );
        HANDLER( handle_pipe );
        HANDLER( handle_pwd );
        HANDLER( handle_quit );
        HANDLER( handle_quit );
        HANDLER( handle_quit );
	HANDLER( handle_read_var );
        HANDLER( handle_repeat );
        HANDLER( handle_rl_info );
        HANDLER( handle_set );
        HANDLER( handle_sleep );
        HANDLER( handle_source );
        HANDLER( handle_system );
        HANDLER( handle_system );
        HANDLER( handle_unset );
        HANDLER( handle_which );
        HANDLER( handle_which );

#if shellish_HAVE_PTHREAD
        HANDLER( handle_background_pthread );
#endif

#if shellish_ENABLE_S11N
        HANDLER( handle_s11n );
#endif

#undef HANDLER

} // namespace shellish


#endif // shellish_BUILTINS_HPP_INCLUDED
