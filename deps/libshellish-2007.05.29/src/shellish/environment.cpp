#include <stdlib.h> // setenv()
#include <unistd.h> // getuid() and friends
#include <sys/types.h>
#include <pwd.h> // /etc/password stuff
#include <string.h> // strlen()

// author: stephan@s11n.net
// license: Public Domain

#include <unistd.h>

#include <s11n.net/shellish/environment.hpp>
#include <s11n.net/shellish/key_value_parser.hpp>
#include <s11n.net/shellish/shellish_debuggering_macros.hpp> // COUT/CERR

using std::string;

extern char **environ; // HTF do you reference that from inside a namespace?
namespace {
        char ** environment_namespace_kludge() {
                return environ;
        }
}
namespace shellish {
        environment::environment() : m_propagate(false) {}
        environment::~environment() {}

        void
        environment::propagate_sets( bool b ) { this->m_propagate = b; }

        /**
           Internal helper to initialize the state of an environment
           object.
        */
        struct environment_initializer
        {
                void operator()( environment & e, char ** environ )
                {
                        e.propagate_sets( true );
                        e.clear_properties();
                        key_value_parser parser;
                        char *cur = 0;
                        for( int i = 0; 0 != (cur = environ[i]) ; i++ )
                        {
                                if( parser.parse( cur ) )
                                {
                                        e.set_string( parser.key(), parser.value() );
                                }
                        }
                }

                void operator()( environment & e )
                {
                        this->operator()( e, ::environment_namespace_kludge() );
                }
        };

        environment &
        environment::env(  char ** envir )
        {
//                 typedef phoenix::phoenix<
//                         environment,
//                         environment,
//                         environment_initializer
//                         > ENV;
//                 environment & e = ENV::instance();
		static environment bob;
		if( (!envir) && bob.empty() )
		{
			environment_initializer()( bob );
		}
                if( envir )
                {
                        environment_initializer()( bob, envir );
                }
                return bob;
        }

        void
        environment::set_string( const std::string &key, std::string val )
        {
                property_store::set_string( key, val );
                if( this->m_propagate )
                {
#define TRY_PUTENV 0
#if TRY_PUTENV
                        std::string s = key + "=" + val;
                        if( 0 != ::putenv( const_cast<char *>( s.c_str() ) ) )
                        {
                                CERR << "WARNING: ::putenv() return !0!\n";
                        }
#else
                        if( 0 != ::setenv( key.c_str(), val.c_str(), 1 ) )
                        {
                                CERR << "WARNING: ::setenv() return !0, meaning not enough space in environment!\n";
                        }
#endif
#undef TRY_PUTENV
                }
        }

        bool
        environment::unset( const std::string &key )
        {
                if( ! this->property_store::unset( key ) ) return false;
                if( this->m_propagate ) ::unsetenv( key.c_str() );
                return true;
        }


        std::string
        environment::expand_vars( const std::string & text ) const
        {
                std::string foo = text;
                this->expand_vars_inline( foo );
                return foo;
        }

        size_t
        environment::expand_vars_inline( std::string & text ) const
        {
                //CERR << "environment::expand_vars(["<<text<<"])"<<std::endl;
                 if( text.size() < 2 ) return 0;

                string::size_type posA = 0, posB = 0;
                static const char vardelim = '$';
                posA = text.find( vardelim );
                if( string::npos == posA )
                {
                        return 0;
                }
                static const char opener = '{';
                static const char closer = '}';
                string tmpvar;
                size_t count = 0;
                static const string allowable_chars =
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_?";
                        // ^^^^^^ why is '?' in the list? So that shellish can support the shell-ish $? var :/.
                char atc;
                posA = text.size() - 1;
                bool slashmode = false;
                for( ; posA >= 0 && posA != string::npos; posA-- )
                {
                        atc = text[posA];
                        if( atc != vardelim )
                        {
                                continue;
                        }
                        if( posA>0 && !slashmode && text[posA-1] == '\\' ) slashmode = true;
                        if( slashmode )
                        {
                                slashmode = false;
                                continue;
                        }
                        posB = text.find_first_not_of( allowable_chars, posA+1 ); // find first non-variablename char
                        if( posB != posA +1 ) posB -= 1;
                        if( posB == string::npos )
                        {
                                posB = text.size() -1;
                        }
                        
                        tmpvar = string();
                        if( posB == posA + 1 ) // ${VAR} or $F, hopefully
                        {
                                atc = text[posB];
                                if( atc != opener )
                                {
                                        // $NONBRACED_VAR
                                        posB = text.find_first_not_of( allowable_chars, posB );
                                        tmpvar = text.substr( posA + 1, posB  );
                                        //CERR << "nonbraced var? ["<<tmpvar<<"]\n";
//                                         tmpvar += atc;
//                                         if( tmpvar.find_first_of( allowable_chars ) != 0 )
//                                         {
//                                                 tmpvar = string();
//                                         }

                                }
                                else // ${VAR}
                                {
                                        //cout << "Activating Bat-parser! atc="<<atc<<" posA="<<posA<<" posB="<<posB << endl;
                                        const size_t maxpos = text.size()-1;
                                        while( atc != closer && posB <= maxpos )
                                        { // extract variable-name part:
                                                atc = text[++posB];
                                                if ( atc != closer ) tmpvar += atc;
                                        }
                                }
                        }
                        else
                        {
                                // extract variable-name part:
                                tmpvar = text.substr( posA+1 /*skip '$'*/, posB-posA );
                        }
                        //CERR << "expand_vars(): tmpvar=["<<tmpvar<<"]"<<endl;
                        if( tmpvar.empty() ) continue;
                        if( ! this->is_set( tmpvar ) ) continue; // don't expand unknown vars to empty strings.
                        tmpvar = this->get_string( tmpvar, "" );
                        //CERR << "expand_vars(): expanded tmpvar=["<<tmpvar<<"]"<<endl;
                        ++count;
                        text.erase( posA, posB - posA +1 );
                        text.insert( posA, tmpvar.c_str() );
                }
                return count;
        }

        bool
        environment::expand_tilde_home( std::string &path ) const
        {
                /**
                   This code mostly taken from http://www.erlenstar.demon.co.uk/unix/faq_3.html
                   
                   Changes by stephan:
                   
                   - take a reference to a non-const string, instead of a const reference.
                   
                   - return a bool instead of a string.
                */
                if (path.empty() || path[0] != '~' ) return false;

                const char *pfx = 0;
                string::size_type pos = path.find_first_of('/');
                if( path.size() == 1 || pos == 1 )
                {
                        pfx = this->get_string( "HOME", "" ).c_str();
                        if( ! pfx )
                        {
                                struct passwd *pw = ::getpwuid( ::getuid() );
                                if( pw ) pfx = pw->pw_dir;
                        }
                }
                else
                {
                        string user( path, 1, (pos == string::npos) ? string::npos : pos -1 );
                        struct passwd * pw = getpwnam( user.c_str() );
                        if( pw ) pfx = pw->pw_dir;
                }
                if( ! pfx ) return false;

                //cout << "pfx="<<pfx<<endl;
                if( pos == string::npos ) // no / in path (e.g., "~nobody")
                {
                        path = string( pfx );
                        return true;
                }

                string result( pfx );
                if( (result.size() == 0) || (result[result.size()-1] != '/') )
                {
                        result += '/';
                }
                //cout << "result="<<result<<endl;

                path.erase( 0, pos + 1);
                path.insert(0, result );
                //result += path.substr( pos +1 );
                return true;
        }


} // namespace

