
// shellish client
#include <s11n.net/shellish/shellish.hpp>
#include <s11n.net/shellish/session.hpp>
#include <s11n.net/shellish/shellish_debuggering_macros.hpp> // CERR


#define APP_VERSION "$Revision: 1.7 $"

void show_app_info()
{
        std::ostream & os = std::cerr;
        os << "shellish generic client. CVS Version " << APP_VERSION << "\n";
        os << shellish_URL << "\n";
        os << std::endl;
}

int main( int argc, char ** argv )
{
	
        typedef shellish::argv_parser ARGV;
        ARGV & args = shellish::init( argc, argv );

        args.set_help( "dl","Lib(s) to load. May be a colon-separated list." );
        args.set_help( "p, prompt STRING","Sets the command prompt." );
        args.set_help( "s, session FILENAME","Sets session data file." );

        if( args.is_help_set() )
        {
                show_app_info();
                std::cout << "Command line options:\n" << args.dump_help() << std::endl;
                return 0;
        }

	// user may pass -s FILENAME or --session=FILENAME to use a specific
	const std::string defsess = shellish::expand( "${HOME}/.shellish.s11n" ); // there's no place like $HOME, eh?
	std::string sessfile = args.get( std::string("s"),
					 args.get( std::string("session"), defsess )
					 );

	try
	{
		CERR << "Loading session file: " << sessfile << "\n";
		shellish::load_session( sessfile );
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
	}
        std::string prompt = args.get( "p", args.get( "prompt", shellish::env().get("SHELLISH_PROMPT", "shellish >" ) ) );
        shellish::env().set( "SHELLISH_PROMPT", prompt );

        std::string dl = args.get( "dl", "" );
        if( ! dl.empty() )
        {
                shellish::dispatch( "dlload " + dl );
        }


        shellish::input_loop( "${SHELLISH_PROMPT}" );
        std::cout << "\n"; // pedantic terminal cleanup :/

	try
	{
		CERR << "Saving session file: " << sessfile << "\n";
		shellish::save_session( sessfile, false );
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
	}

        return 0;
}
