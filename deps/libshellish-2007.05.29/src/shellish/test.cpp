
// proof-of-concept code for shellish
#include <s11n.net/shellish/shellish.hpp>
#include <s11n.net/shellish/shellish_config.hpp>
#include <s11n.net/shellish/shellish_debuggering_macros.hpp> // CERR

/**
*/
int test_handler( const shellish::arguments & args )
{
        CERR << "test_handler():\n";
	shellish::arguments acp = args;
        for( size_t i = 0; i < args.argc(); i++ )
        {
                CERR << "args["<<i<<"] = ["<<args[i]<<"]\n";
        }
	CERR << "args via shift():\n";
	std::string a = acp.shift();
	while( ! a.empty() )
	{
		CERR << "shift() == " << a << "\n";
		a = acp.shift();
	}
        return shellish::ErrorCommandNotMapped;
}





int main( int argc, char ** argv )
{
        typedef shellish::argv_parser ARGV;
        ARGV & args = shellish::init( argc, argv );


        if( args.is_help_set() )
        {
                CERR << "Options:\n" << args.dump_help() <<"\n";
                return 0;
        }

        shellish::map_commander( "*", test_handler, "Test handler." );

        if( ! shellish::env().is_set( "SHELLISH_PROMPT" ) )
        {
                shellish::env().set( "SHELLISH_PROMPT", args.get( "prompt", "shellish >" ) );
        }

	CERR << "dollar-home = "<<shellish::expand("${HOME}") << "\n";
	std::string bogo( "~/something" );
	shellish::expand_tilde_home( bogo );
	CERR << "tilde-home/something = "<<bogo << "\n";

        shellish::input_loop( "${SHELLISH_PROMPT}" );
        shellish::ostream() << "\n"; // pedantic terminal cleanup :/

        return 0;
}
