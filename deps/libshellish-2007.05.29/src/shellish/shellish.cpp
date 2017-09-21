#include <sys/stat.h> // mkdir()
#include <sys/types.h> // mkdir()
#include <stdlib.h> // size_t


#include <algorithm> // copy, replace
#include <iterator> // copy
#include <iostream>
#include <memory> // auto_ptr
#include <fstream> // istream

#include <s11n.net/shellish/shellish.hpp>
#include <s11n.net/shellish/shellish_config.hpp>
#include <s11n.net/shellish/shellish_debuggering_macros.hpp>

#include <s11n.net/shellish/path_finder.hpp>
#include <s11n.net/shellish/strtool.hpp> // trim_string()
#include <s11n.net/shellish/stdstring_tokenizer.hpp>
#include <s11n.net/shellish/plugin.hpp> // opening dlls

#if shellish_HAVE_LIBREADLINECPP
#    include <s11n.net/readline/Readline.hpp>
// #    include <s11n.net/readline/Readline_s11n.hpp>
#endif

#define SHELLISHDEBUG if( shellish_debug_enabled ) CERR



namespace shellish {

	std::string shellish_version()
	{
		return shellish_SHELLISH_VERSION;
	}

	int dispatcher::dispatch( const arguments & args )
	{
		commander_map::const_iterator cit = this->handler_map().find( args[0] );
		commander_map::const_iterator cet = this->handler_map().end();
		if( cet == cit )
		{
			cit = this->handler_map().find( "*" );
		}
		if( cet == cit )
		{
			return ::shellish::ErrorCommandNotMapped;
		}
		return (*cit).second( args );
	}



        /**
         Internal use. Returns THE default dispatcher.
        */
        dispatcher & absolute_default_dispatcher()
        {
                static dispatcher bob;
                return bob;
        }


        dispatcher_map & dispatchers()
        {
                static dispatcher_map bob;
                if( bob.empty() )
                {
                        bob["shellish-default"] = &absolute_default_dispatcher();
                }
                return bob;
        }


        dispatcher * get_dispatcher( const std::string & name )
        {
                dispatcher_map::iterator it = dispatchers().find( name );
                if( dispatchers().end() == it ) return 0;
                return (*it).second;
        }

        void add_dispatcher( const std::string & name, dispatcher * dist )
        {
                dispatchers()[name] = dist;
        }

        dispatcher * shellish_dispatcher = 0;
        dispatcher & default_dispatcher()
        {
                return shellish_dispatcher
                        ? *shellish_dispatcher
                        : absolute_default_dispatcher();
        }

        void default_dispatcher( dispatcher * d )
        {
                shellish_dispatcher = d;
        }

#define HMAP (default_dispatcher().handler_map())

        bool is_comment_line( const std::string & str )
        {
		if( str.empty() ) return false;
		return str.find_first_of( "#" ) < str.find_first_not_of( " \t#" );
        }

        bool shellish_debug_enabled = false;

        std::ostream * shellish_ostream = &std::cout;
        std::ostream * shellish_errstream = &std::cerr;

        struct shellish_sharing_context {}; // private type for use with phoenix<>


        argv_parser & init( int argc, char ** argv )
        {
                argv_parser & a = argv_parser::args( argc, argv );

                if( a.is_set( "shellish-debug" ) )
                {
                        shellish_debug_enabled = true;
                        SHELLISHDEBUG << "Enabling shellish-debug!"<<std::endl;
                }

		const std::string ckey = "shellish-classpath";
		path_finder & cp = plugin::path();
		cp.add_path( expand(shellish_SHARED_LIB_PATH) );
		if( a.is_set( ckey ) )
		{
			cp.add_path( a.get(ckey,"ERROR") );
		}
		SHELLISHDEBUG << "classpath="<<cp.path_string()<<"\n";
                setup_default_handlers();
                return a;
        }

        environment & env()
        {
                return environment::env();
        }

        argv_parser & args()
        {
                return argv_parser::args();
        }

        aliaser & aliases()
        {
		static aliaser bob;
		return bob;
//                 return phoenix::phoenix<aliaser,commander_map>::instance();
       }


	bool expand_tilde_home_inline( std::string & token )
	{
		return env().expand_tilde_home( token );
	}

	std::string expand_tilde_home( const std::string & token )
	{
		std::string r = token;
		expand_tilde_home_inline( r );
		return r;
	}



        typedef std::map<std::string,std::string> help_map_type;
        help_map_type & help()
        {
                //return phoenix::phoenix<help_map_type,commander_map>::instance();
		static help_map_type bob;
		return bob;
        }

        void map_help( const std::string & key, const std::string & text )
        {
                help()[key] = text;
        }

        void map_commander( const std::string & c, command_handler_func h, const std::string & text )
        {
                HMAP[c] = h;
                SHELLISHDEBUG << "Mapping handler for '"<<c<<"'.\n";
                if( ! text.empty() ) map_help( c, text );
        }


        int shellish_debug_toggle( const arguments & )
        {
                std::ostream & os = ::shellish::ostream();
                shellish_debug_enabled = !shellish_debug_enabled;
                os << "shellish internal debugging " << (shellish_debug_enabled ? "on" : "off") << "." << std::endl;
                return 0;
        }

        int dispatch( const std::string & command )
        {
                if( 0 == command.size() ) return ErrorNoArguments;
		if( is_comment_line( command ) ) return CommentLine;
		std::string c = strtool::trim_string(expand( aliases().expand( command ) ) ) ;
                return dispatch( arguments( c ) );
        }


        int dispatch( const arguments & args )
        {
                if( 0 == args.argc() ) return ErrorNoArguments;
		if( is_comment_line( args[0] ) ) return CommentLine;
                if( env().get_bool( "echo", false ) )
                {
                        ::shellish::ostream() << args.str() << std::endl;
                }
                int code = ErrorException;
		try
		{
			code = default_dispatcher().dispatch( args );
		}
		catch(const std::exception &ex)
		{
			errstream() << "shellish::dispatch() caught an exception: " << ex.what() << "\n";
		}
		catch(...)
		{
		}
                env().set( "?", code );
                return code;
        }


        int dispatch( std::istream & is, bool stopOnError )
        {
                std::ostream & os = ::shellish::ostream();
                std::string line;
                int ret = 0;
                size_t lc = 0;
                while( std::getline( is, line ) )
                {
                        ++lc;
                        if( (std::string::npos == line.find_first_not_of(" \t")) /* empty line? */
			    || is_comment_line( line )
			    )
			{
				continue;
			}
                        if( 0 != (ret = dispatch( line ) ) )
                        {
                                os << "shellish: dispatch(istream) failed [code="
				   <<ret<<"] on line " << lc << ". Input was:\n"
				   << line << "\n";
				if( stopOnError )
				{
					return ret;
				}
                        }
                }
                return ret;
        }


#if shellish_HAVE_LIBREADLINECPP
        readlinecpp::Readline & readline()
        {
//                 typedef phoenix::phoenix<
//                         readlinecpp::Readline,
//                         shellish_sharing_context
//                         > PHX;
//                 return PHX::instance();
		static readlinecpp::Readline bob;
		return bob;
        }
#endif // shellish_HAVE_LIBREADLINECPP

        std::string read_line( const std::string & prompt, bool & breakout )
        {
                std::string realprompt = env().expand_vars( prompt );
                std::string str;
#if shellish_HAVE_LIBREADLINECPP
                str = readline().readline( realprompt, breakout );
#else
                std::cout << realprompt;
                breakout = std::getline( std::cin, str ).eof();
#endif // shellish_HAVE_LIBREADLINECPP
                return breakout ? "" : str;
        }

        int handle_prompt_for_input( const shellish::arguments & _args )
        { // default implementation for shellish_BUILTIN_PROMPT_FOR_INPUT
                // std::ostream & os = ::shellish::ostream();
                shellish::arguments args = _args;
		args.shift();
		std::string prompt = args.str();
		if( prompt.empty() )
		{
			prompt = env().get( "SHELLISH_PROMPT", ">" );
		}
		bool endin;
		std::string line  = read_line( prompt, endin );
		if( endin )
		{
			end_input_loop();
			// we don't actually know if it's running,
			// but we need to turn it off now if it is.
		}
		if( ! line.empty() )
		{
			env().set( shellish_ENVVAR_READLINE_INPUT, line );
		}
		else
		{
			env().unset( shellish_ENVVAR_READLINE_INPUT );
		}
		return 0;
	}

	std::string prompt_for_input( const std::string & prompt )
	{
		std::string pr = prompt;
		// kludge == workaround to keep the $? var intact
		// when we proxy the input fetch through the dispatch()
		// process.
		int kludge = env().get<int>("?",(int)0);
		dispatch( shellish_BUILTIN_PROMPT_FOR_INPUT + " " + pr );
		env().set("?",kludge);
		return env().get( shellish_ENVVAR_READLINE_INPUT, std::string() );
	}

        bool shellish_continue_input = true; // when set to false, input_loop() will stop looping.

        void input_loop( const std::string & prompt )
        {
                std::ostream & os = ::shellish::ostream();
                std::string line;
                std::string cmd;
                int err;
                shellish_continue_input = true;
                do
                {
			line = prompt_for_input( prompt );
			if( ! shellish_continue_input ) break;
			// env().unset( shellish_ENVVAR_READLINE_INPUT );
                        if( line.empty() )
                        {
                                continue;
                        }
                        SHELLISHDEBUG << "input line: " << line<<std::endl;
			// BUG: the following input line gets its quotes stripped incorrectly:
			//  command arg1 arg2; command2 arg1 "arg two"
			// i think the bug lives here, but i haven't yet looked into it all that
			// closely.
                        strtool::stdstring_tokenizer tok( line, ";" );
                        while( tok.has_tokens() )
                        {
                                cmd = tok.next_token();
                                strtool::trim_string( cmd );
                                SHELLISHDEBUG << "dispatching ["<<cmd<<"]\n";
                                err = dispatch( cmd );
                                if( 0 != err )
                                {
                                        os << "[error code="<<std::dec<<err<<"]"<<std::endl;
                                }
                        }
                }
                while( shellish_continue_input );
                return;
        }


        /**
           Tells input_loop() to stop processing.
        */
        void end_input_loop()
        {
                shellish_continue_input = false;
        }


        int handle_dlload( const shellish::arguments & _args )
        {
                std::ostream & os = ::shellish::ostream();

                shellish::arguments args = _args;
		path_finder & pf = plugin::path();
                if( 1 == args.argc() )
                {
                        os << "Usage: " << args[0] << " dll_name" << "\n"
			   << "dll_name may be a basename if dll_name.so is "
			   << "found in the lib path, otherwise it must be an absolute path."
			   << "\nLib path is currently:\n"
			   << pf.path_string()
			   << std::endl;
                        return ::shellish::ErrorUsageError;
                }
                args.shift(); // drop args[0]
                std::string dlname;
                std::string p;
                strtool::stdstring_tokenizer tok;
                arguments::const_iterator cit = args.begin();
                arguments::const_iterator cet = args.end();
                for( ; cit != cet; ++cit )
                {
                        dlname = (*cit);
                        // Accept delimited paths to ease implementation
                        // of 'shellish' client app:
                        tok = strtool::stdstring_tokenizer( dlname, ":" );
                        while( tok.has_tokens() )
                        {
                                dlname = tok.next_token();
                                try
                                {
                                        p = ::shellish::plugin::open( dlname );
                                }
                                catch( const std::exception & ex )
                                {
                                        os << "Caught std::exception while loading plugin '"<<dlname<<"': " << ex.what() << "\n";
                                        return ::shellish::ErrorException;
                                }
                                catch( ... )
                                {
                                        os << "Caught unknown exception while loading plugin '"<<dlname<<"'.";
                                        return ::shellish::ErrorException;
                                }
                                if( p.empty() )
                                {
                                        os << "Error: no DLL found for '"<<dlname<<"', or opening it failed." << std::endl;
                                        return ::shellish::ErrorResourceNotFound;
                                }
                                os << "Opened DLL: " << p<<std::endl;
                        }
                }
                return 0;
        }

        int handle_help( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                typedef shellish::commander_map::const_iterator MAPIT;
                MAPIT mit = HMAP.begin();
                MAPIT met = HMAP.end();
                typedef shellish::help_map_type::const_iterator HI;
                std::string cmd = args[1];
                HI hit = shellish::help().find( cmd ),
                        het = shellish::help().end();
                if( het != hit )
                { // volltreffer!
                        os << "Help text for '" << cmd<<"':\n" << (*hit).second << std::endl;
                        return 0;
                }
                os << "Mapped commands: " << std::endl;
                std::string key;
                for( ; met != mit; ++mit )
                {
                        key = (*mit).first;
                        if( "*" == key ) continue;
                        os << "["<<key<<"]";
                        hit = help().find( key );
                        if( het != hit )
                        {
                                os << ":    " << (*hit).second;
                        }
                        os << std::endl;
                }
                return 0;
        }

        int handle_history( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
#if shellish_HAVE_LIBREADLINECPP
                std::copy( readline().history().begin(), readline().history().end(),
                           std::ostream_iterator<std::string>( os, "\n" ) );
#else
                os << "warning: shellish was built without libreadline_cpp, so command history is not enabled." << std::endl;
#endif // shellish_HAVE_LIBREADLINECPP
                return 0;
        }


        void setup_default_handlers()
        {
                map_commander( "dlload", handle_dlload, "Open DLL $1. May be a basename, if $1.so is in lib path, or absolute path." );
                map_commander( shellish_BUILTIN_PROMPT_FOR_INPUT, handle_prompt_for_input, "(internal) Prompts the user for a line of input. Result goes to $"+shellish_ENVVAR_READLINE_INPUT+"." );


                // TODO: move help/history out of core and into builtins lib.
                // They are here because they have close ties to some core data.
                map_commander( "help", handle_help, "Shows command help." );
                map_commander( "?", handle_help, "Same as 'help'." );
                map_commander( "history", handle_history, "Shows command history." );

                std::string autoload = env().get( "SHELLISH_AUTOLOAD_LIBS", shellish_AUTOLOAD_LIBS );
		if( ! autoload.empty() )
		{
			dispatch( "dlload " + autoload );
		}
        }


        std::string expand( const std::string & in )
        {
                if( in.empty() ) return in;
                std::string ret;
                std::string str = in;
                env().expand_vars_inline( str );
                SHELLISHDEBUG << "expand(["<<in<<"]) == ["<<str<<"]\n";
		// Reminder, in case you reconsider ::wordexp():
		// $vars expansion by ::wordexp() does not expand
		// those set via env().set(), even though that calls
		// ::setenv(). We must expand those before ::wordexp()
		// does, so that vars it doesn't know about aren't
		// expanded to null.
		return str;
        }

        void ostream( std::ostream * os )
        {
                shellish_ostream = (os ? os : &std::cout);
        }

        std::ostream & ostream()
        {
                return *shellish_ostream;
        }

        void errstream( std::ostream * os )
        {
                shellish_errstream = (os ? os : &std::cerr);
        }

        std::ostream & errstream()
        {
                return *shellish_errstream;
        }





} // namespace

#undef HMAP
