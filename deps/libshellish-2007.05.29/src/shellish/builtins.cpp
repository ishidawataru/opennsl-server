#include <unistd.h> // sleep()

#include <fstream>
#include <algorithm>

#include <s11n.net/shellish/builtins.hpp>
#include <s11n.net/shellish/shell_process.hpp>

#include <s11n.net/shellish/key_value_parser.hpp>
#include <s11n.net/shellish/environment.hpp>
#include <s11n.net/shellish/aliaser.hpp>
#include <s11n.net/shellish/path_finder.hpp>
#include <s11n.net/shellish/shellish_debuggering_macros.hpp>

#if shellish_HAVE_LIBREADLINECPP
#    include <s11n.net/readline/Readline.hpp>
// #    include <s11n.net/readline/Readline_s11n.hpp>
#endif


#define SHELLISH_USE_SIGNALS 1
#if SHELLISH_USE_SIGNALS
#  include <stdlib.h> // atexit()
#  include <signal.h> // signal()
#  include <setjmp.h> // longjmp()
#endif


#if shellish_HAVE_PTHREAD
#  include <pthread.h>     /* pthread functions and data structures */
#endif

#if shellish_ENABLE_S11N
#include <s11n.net/s11n/s11nlite.hpp>
#endif

namespace shellish {

        void init_shellish_builtins()
        {
                /*
                map_commander( "!", handle_system, "Same as 'system'." );
                map_commander( "alias", handle_alias, "As in Unix shells: alias X=\"something else\"" );
                map_commander( "cd", handle_chdir, "Changes current working directory." );
                map_commander( "chdir", handle_chdir, "Same as 'cd'." );
                map_commander( "echo", handle_echo, "Echoes it's arguments." );
                map_commander( "env", handle_env, "Lists all environment variables." );
                map_commander( "shellish-debug-toggle", shellish_debug_toggle, "Toggles internal shellish debuggering output on or off." );
                map_commander( "shellish-info", handle_shellish_info, "Shows some info about shellish." );
                map_commander( "exit", handle_quit, "Ends interactive input mode." );
                map_commander( "fork", handle_fork, "Runs it's arguments via fork()/exec()." );
                map_commander( "logout", handle_quit, "Same as 'exit'." );
                map_commander( "pipe", handle_pipe, "Runs arguments through C's pipe() function." );
                map_commander( "pwd", handle_pwd, "Echoes current working directory's name." );
                map_commander( "quit", handle_quit, "Same as 'exit'." );
                map_commander( "readvar", handle_read_var, "Reads user input into a $VAR: readvar VARNAME [...VARNAME_N]" );
                map_commander( "repeat", handle_repeat, "Calls dispatch() $1 times on $2..$N." );
                map_commander( "input-info", handle_rl_info, "Shows which line input implementation shellish uses." );
                map_commander( "set", handle_set, "Sets an environment variable: set VAR=VALUE" );
                map_commander( "sleep", handle_sleep, "Sleeps for a given number of seconds. If $3==1 then count down of the seconds is shown." );
                map_commander( "source", handle_source, "Processes a script containing dispatch()-able commands." );
                map_commander( "system", handle_system, "Runs it's arguments through C's system() function." );
                map_commander( "unset", handle_unset, "Unsets the environment variables passed to it." );
                map_commander( "which", handle_which, "Finds a file ($1) in a given PATH ($2), defaulting to ${PATH}." );

#if shellish_HAVE_PTHREAD
                map_commander( "bgthread", handle_background_pthread, "Runs $2..$N in a background thread." );
#endif
#if shellish_ENABLE_S11N
                map_commander( "s11n", handle_s11n, "set/view s11nlite info." );
#endif         */               
        }

        int handle_shellish_info( const arguments & )
        { // displays shellish lib info
		::shellish::ostream() << "shellish info:\n"
				    << "-- Version: " << shellish_version() << "\n"
				    << "-- Using libreadline_cpp? " << (shellish_HAVE_LIBREADLINECPP ? std::string("yes :)") : std::string("no :(")) <<"\n"
				    << "-- Sessions support enabled? " << (shellish_ENABLE_S11N ? std::string("yes :)") : std::string("no :(")) <<"\n"
				    << "\n";
		return 0;
	}


        static int bogus_init_var = (init_shellish_builtins(),0);


        int handle_rl_info( const arguments & )
        { // displays Readline lib info
		::shellish::ostream() << "Line-based input implementation: "
#if shellish_HAVE_LIBREADLINECPP
				    << ::readlinecpp::Readline::rl_implementation_name()
#else
				    << "stdin"
#endif
				    << std::endl;
		return 0;
	}

	int handle_read_var( const ::shellish::arguments & _args )
	{ // $1..$N = VARNAMEs
		std::ostream & os = ::shellish::ostream();
		shellish::arguments args = _args;
		args.shift(); // pop arg0
		if( 0 == args.argc() )
		{
			os << "Usage error. Try: "<<_args[0]<<" VARIABLE_NAME [...VARIABLE_NAME_N]\n";
			return shellish::ErrorUsageError;
		}
		typedef shellish::arguments::const_iterator CIT;
		CIT it = args.begin();
		CIT et = args.end();
		std::string var;
		std::string s;
		int count = 0;
		for( ; et != it; ++it )
		{
			var = (*it);
			s = shellish::prompt_for_input( var + "=" );
			if( ! s.empty() )
			{
				++count;
				s = shellish::expand( s ); // does $ expansion
				shellish::env().set( var, s );
			}
		}
		return 0;
	}


#if shellish_HAVE_PTHREAD
        void * handle_background_pthread_run( void * data )
        {
                arguments * a = (arguments *)data; // HOPE!
                int ret = dispatch( *a );
                shellish::ostream() << "Background thread ["<<a->str()<<"] finished with return code " << ret << ".\n";
                delete( a );
                data = 0;
                return NULL;
        }


        int handle_background_pthread( const arguments & a )
        { // launches args in a new thread
                std::ostream & os = shellish::ostream();
                if( 2 > a.argc() )
                {
                        std::string a0 = a[0];
                        os << "Usage error. Try: " << a0 << " command [args...]\n"
                           << "e.g., "<<a0<<" sleep 1000 1\n";
                        return ::shellish::ErrorUsageError;
                }
                arguments * args = new arguments(a); // new thread will own it.
                args->shift();
                std::string cmd = args->str();
                pthread_t thr;
                pthread_attr_t * attr = new pthread_attr_t;
                pthread_attr_setdetachstate( attr, PTHREAD_CREATE_DETACHED );
                os << "Backgrounding ["<<cmd<<"].\n";
                int ret = pthread_create( &thr, attr, handle_background_pthread_run, args );
                // i'm not clear at all on the ownership of attr on a detached thread.
                // if i don't create it with new() i get segfaults sometimes in
                // pthread_create(). But i get the feeling i shouldn't delete it, either.
                // Deleting it also segfaults
                delete attr;

                if( 0 != ret )
                {
                        os << "Error starting new thread!\n";
                        delete args;
                        return ::shellish::Error;
                }
                return 0;
        }

#endif // shellish_HAVE_PTHREAD


        int handle_quit( const arguments & )
        { // tells input_loop() to stop.
                end_input_loop();
                return EShellExit;
        }

        int handle_echo( const arguments & args )
        { // like shell's echo ...
                std::ostream & os = ::shellish::ostream();
                arguments::args_list::const_iterator cit = args.argv().begin();
                arguments::args_list::const_iterator cet = args.argv().end();
                bool donl = true;
                ++cit; // skip argv0
                for( ; cet != cit; ++cit )
                {
                        if( donl && ("-n" == (*cit)) )
                        {
                                donl = false;
                                continue;
                        }
                        os << (*cit) << " ";
                }
                if( donl ) os << std::endl;
                return 0;
        }

        int handle_set( const arguments & args )
        { // like shell's "setenv foo=bar"
                std::ostream & os = ::shellish::ostream();
                if( 2 > args.argc() )
                {
                        os << "set: usage: set key=val" << std::endl;
                        return ErrorUsageError;
                }
                arguments::args_list::const_iterator cit = args.argv().begin();
                ++cit; // discard arg0
                key_value_parser kp;
//                 CERR << "setting: "<<(*cit)<<"\n";
                if( ! kp.parse( *cit ) )
                {
                        os << "set: unparseable tokens: ["<<(*cit)<<"]"<< std::endl;
                        return ErrorParseError;
                }
                env().set( kp.key(), kp.value() );
                return 0;
        }

        int handle_unset( const arguments & args )
        { // like shell's "unset foo"
                std::ostream & os = ::shellish::ostream();
                if( 2 > args.argc() )
                {
                        os << "unset: usage: set key=val" << std::endl;
                        return ErrorUsageError;
                }
                arguments::args_list::const_iterator cit = args.argv().begin();
                arguments::args_list::const_iterator cet = args.argv().end();
                ++cit; // discard arg0
                for( ; cit != cet; ++cit )
                {
                        env().unset( *cit );
                }
                return 0;
        }

        int handle_env( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                environment::map_type::const_iterator it = env().begin();
                environment::map_type::const_iterator et = env().end();
                for( ; it != et; ++it )
                {
                        os << (*it).first << "=" << (*it).second << std::endl;
                }
                return 0;
        }

        int handle_chdir( const arguments & args )
        { // like shell's cd ...
                std::ostream & os = ::shellish::ostream();
                arguments::args_list::const_iterator cit = args.argv().begin();
                ++cit; // discard argv0
                std::string arg;
                if( cit == args.argv().end() )
                {
                        arg = env().get_string( "HOME" );
                }
                else
                {
                        arg = *cit;
                        if( "-" == arg )
                        {
                                arg = env().get_string( "OLDPWD" );
                        }
//                         if( "~" == arg || "~/" == arg )
//                         {
//                                 arg = env().get_string( "HOME" );
//                         }
                }
                std::string oldpwd = env().get_string("PWD");
		::shellish::expand_tilde_home( arg );
                int ret = ::chdir( arg.c_str() );
                if( 0 == ret )
                {
                        const int bsize = 512;
                        char buff[bsize];
                        getcwd( buff, bsize );
                        std::string swd = std::string(buff);
                        env().set( std::string("PWD"), swd );
                        env().set( "OLDPWD",oldpwd );
                        os <<swd<<std::endl;
                }
                else
                {
                        os << "cd " << arg << " failed!" << std::endl;
                        return ret;
                }
                return ret;
        }

        int handle_pwd( const arguments & args )
        { // like shell's pwd ...
                std::ostream & os = ::shellish::ostream();
                os << env().get( "PWD", "???" ) << std::endl;
                return 0;
        }

        int handle_alias( const arguments & args )
        { // sets/unsets alias. Set to an empty value to unset.
                std::ostream & os = ::shellish::ostream();
                typedef aliaser::map_type::const_iterator MAPIT;
                if( 1 == args.argc() )
                {  // list aliases
                        MAPIT cit = aliases().map().begin();
                        MAPIT citend = aliases().map().end();
                        for( ; cit != citend; cit++ )
                        {
                                os << (*cit).first << "=\""<<(*cit).second<<"\"\n";
                        }
                        return 0;
                }

                arguments::args_list::const_iterator ait = args.argv().begin();
                std::string arg0 = *ait;
                ++ait;
                std::string rest = *ait;

                if( std::string::npos != rest.find_first_of( "=" ) )
                { // alias foo=bar
                        key_value_parser kp;
                        if( ! kp.parse( rest ) )
                        {
                                os << "alias: unparseable tokens: ["<<rest<<"]"<< std::endl;
                                return ErrorParseError;
                        }
                        aliases().alias( kp.key(), kp.value() );
                        //shellish::map_help( kp.key(), "Alias for [" + kp.value() + "]" );
                        return 0;
                }

                MAPIT cit = aliases().map().find(rest);
                if( cit == aliases().map().end() )
                {
                        CERR << "Not aliased: '"<<rest<<"'"<<std::endl;
                        return Error;
                }
                os << (*cit).first << "="<< (*cit).second << std::endl;
                return 0;
        }

        int handle_fork( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                arguments pass = args;
                pass.shift();
                return shellish::shell_process().fork( pass.str(), os );
        }
        int handle_pipe( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                arguments pass = args;
                pass.shift();
                return shellish::shell_process().pipe( pass.str(), os );
        }
        int handle_system( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                arguments pass = args;
                pass.shift();
                return shellish::shell_process().system( pass.str(), os );
        }


#if SHELLISH_USE_SIGNALS
        sigjmp_buf shellish_ctrl_c_jmp_buffer;
        void shellish_SIGINT(int)
        {
                std::cout << "^C" << std::endl; // this really shouldn't be here, but i find it useful.
                siglongjmp( shellish_ctrl_c_jmp_buffer, 1 );
        }
#endif // #if SHELLISH_USE_SIGNALS


        int
        handle_sleep( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                arguments a = args;
                std::string arg = a.shift();
                int myint = strtool::from<int>( a.shift(), 0 );
                if( 0 >= myint )
                {
                        os << "Error: time to sleep to small. You must use a number > 0." << std::endl;
                        return 1;
                }
                int noisy = strtool::from<int>( a.shift(), 0 );
#if SHELLISH_USE_SIGNALS
                typedef void (*signalfunc)(int);
                signalfunc old_sighandler = ::signal( SIGINT, shellish_SIGINT );
                if( 0 == sigsetjmp( shellish_ctrl_c_jmp_buffer, 1 ) )
                {
#endif // SHELLISH_USE_SIGNALS
                        // if we don't do SIGINT handling here then
                        // the app is left in a useless state if
                        // Ctrl-C is pressed during ::sleep().
                        if( noisy != 0 )
                        {
                                for(int i = myint; i > 0; i--)
                                {
                                        os << i << " ";
                                        os.flush();
                                        ::sleep(1);
                                }
                                os << std::endl;
                                return 0;
                        }
                        return ::sleep( myint );
#if SHELLISH_USE_SIGNALS
                }
                else // returning from a Ctrl-C
                {
                        ::signal( SIGINT, old_sighandler );
                }
#endif // SHELLISH_USE_SIGNALS
                return 0;
        }


        /** Similar to bash's built-in 'which' */
        int handle_which( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                arguments acp = args;
                acp.shift(); // drop $0
                std::string arg = acp.shift();
                if( arg.empty() )
                {
                        os << "usage: which FILE [PATH]" << std::endl;
                        return shellish::ErrorUsageError;
                }
                std::string path = acp.shift();
                if( path.empty() )
                {
                        path = shellish::env().get_string( "PATH" );
                }
                path_finder pf( path );
                std::string f = pf.find( arg );
                if( f.empty() )
                {
                        os << "'"<<arg<<"' not found in path ["<<path<<"]" << std::endl;
                        return shellish::ErrorResourceNotFound;
                }
                os << f << std::endl;
                return 0;
        }




        int handle_repeat( const arguments & _a )
        {
                std::ostream & os = ::shellish::ostream();
                arguments a(_a);
                a.shift(); // pop $0
                int num = strtool::from<int>( a.shift(), -1 );
                if( num <= 0 )
                {
                        os << "Usage error: $1 must be a positive integer." << std::endl;
                        return ::shellish::ErrorUsageError;
                }
                int ret = 0;
		arguments theargs;
		arguments aliex( aliases().expand( a[0] ) );
		if( aliex.argc() > 1 )
		{ // token expanded to more than 1 token, so we need
		  // to do a bit of fudging here.
			std::copy( aliex.begin(), aliex.end(),
				   std::back_inserter( theargs.argv() ) );
			arguments::iterator it = a.begin();
			if( it != a.end() ) ++it;
			std::copy( it, a.end(),
				   std::back_inserter( theargs.argv() ) );
		}
		else
		{
			theargs = a;
		}
                for( int i = 0; i < num; i++ )
                {
                        ret = shellish::dispatch( theargs );
                }
                return ret;
        }

        int
        handle_source( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                std::string fname = args[1];
                if( fname.empty() )
                {
                        os << "Usage error: $1 must be a filename containing shellish-able commands." << std::endl;
                        return ::shellish::ErrorUsageError;
                }
 		std::ifstream is( fname.c_str() );
//                 typedef std::auto_ptr<std::istream> AP;
//                 AP is( ::s11n::io::get_istream( fname ) ); // might support compressed files
                if( ! is.good() )
                {
                        os << "Unknown error reading file '"<<fname<<"'." << std::endl;
                        return ::shellish::ErrorResourceNotFound;

                }
                return ::shellish::dispatch( is );
        }


#if shellish_ENABLE_S11N
        int
        handle_s11n( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
		arguments av(args);
		std::string a0 = av.shift();
                std::string cmd = av.shift();

		if( cmd == "serializer" )
		{
			std::string sc = av.shift();
			if( sc.empty() )
			{
				os << a0 << " " << cmd << " requires a serializer class name.\n";
				return ::shellish::ErrorUsageError;
			}
			s11nlite::serializer_class( sc );
			os << "s11nlite serializer class set to: " << sc << "\n";
			return 0;
		}

		if( cmd == "info" )
		{
			os << "s11n library version: " << s11n::library_version() << "\n"
			   << "default s11nlite serializer class: " << s11nlite::serializer_class() << "\n"
				;
			return 0;
		}
                os << "Usage error: try one one:\n"
		   << "\t"<<a0 << " serializer CLASSNAME\n"
		   << "Sets the s11nlite serializer class.\n"
		   << "\t"<<a0 << " info\n"
		   << "Shows s11nlite info.\n"
		   <<"\n";
		return ::shellish::ErrorUsageError;
        }

#endif // shellish_ENABLE_S11N




} // namespace shellish
