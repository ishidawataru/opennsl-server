// Copyright (C) 2002, 2003 stephan@s11n.net
// Released under the See LICENSE file in this directory.

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <stdio.h> // sprintf()
#include <assert.h>


#include <s11n.net/shellish/key_value_parser.hpp>
#include <s11n.net/shellish/argv_parser.hpp>
#include <s11n.net/shellish/stdstring_tokenizer.hpp>


namespace shellish {
        argv_parser::argv_parser() 
        {
        }
        argv_parser::argv_parser( int argc, char *argv[], int startAt )
        {
                args( argc, argv, startAt );
        }

        argv_parser::~argv_parser()
        {
        }


        void
        argv_parser::set_help( const std::string &key, const std::string &text )
        {
                this->helpmap.set_string( key, text );
        }

        const std::string
        argv_parser::get_help( const std::string &key ) const
        {
                std::string help = helpmap.get_string( key );
                return help;
        }


        std::string
        argv_parser::get_string( const std::string &key, const std::string & defaultVal ) const
        {
                // DO NOT call LIBE_{DEBUG,VERBOSE} from here, or you'll cause an endless loop.
                std::string check = property_store::get_string( key, defaultVal );
    
                if( check != defaultVal ) return check;

                if( key.find( "-" ) != 0 ) // non-dashed argument
                {
                        // now try -key, --key
                        check = key;
                        std::string foo;
                        for( int i = 0; i < 2; i++ )
                        {
                                check.insert( check.begin(), '-' );
                                foo = property_store::get_string( check, defaultVal );
                                //std::cerr << "dash test: " << check << " = " << foo << endl;
                                if( foo != defaultVal ) return foo;
                        }
                }
                return defaultVal;
        }



        int
        argv_parser::args( int argc, char *argv[], int startAt, const char *argpre )
        {
                using namespace std;
                if( startAt >= argc ) return -1;
                if( ! argv[startAt] ) return -1;
                std::string argprefix = argpre ? argpre : "-";
                int acount = 0;
                std::string v;
                std::string a;
                key_value_parser kvp;
                std::string nextarg;
                static const string numbers = "0123456789";
                bool skipnext = false;
                char * dollarstring = new char[10];
//                 std::string accum;
		this->set( "$0", std::string(argv[0]) ); // shell/perl-style
                for( int i = startAt; i < argc; i++ )
                {
                        ::snprintf( dollarstring,10,"$%d", i );
                        this->set_string( dollarstring, argv[i] );
    
//                         accum += argv[i];
//                         if( argc > i-1 ) accum += ' ';

                        if( skipnext ) { skipnext = false; continue; }
                        a = std::string( (const char *)argv[i] );
                        //CERR << "arg="<<a<<endl;
                        if( a.find(argprefix)!=0 ) continue;
                        v = std::string();
                        ++acount;
                        if( kvp.parse( a ) ) // check for: --foo=bar
                        {
                                a = kvp.key();
                                v = kvp.value();
                        }
                        else // else it's space-separated or a boolean flag
                        {
                                if( i < argc-1 )
                                {
                                        nextarg = std::string(argv[i+1]);
                                        if( nextarg == "-" )
                                        {
                                                // workaround for things like: myapp --infile -
                                                v = "-";
                                                skipnext = true;
                                        }
                                        else if( nextarg.find(argprefix) == 0 )
                                        {
                                                if( nextarg.find_first_of( numbers ) == 1 )
                                                { // let's assume it's a negative number, not a flag
                                                        // todo: actual atol() or atod() check
                                                        // Note that this logic means that numbers as flags is not supported. :/
                                                        skipnext = true;
                                                        v = nextarg;
                                                }
                                                else
                                                { // nextarg is argprefix'd, so treat this as a boolean flag
                                                        v = "true";
                                                }
                                        }
                                        else
                                        { // it was space-separated: --foo bar
                                                v = nextarg;
                                                skipnext = true;
                                        }
                                }
                                else
                                {
                                        v = "true"; // the final item is an argument, toggling it to true.
                                }
                        }
                        this->set( a, v );
                        //CERR << "["<<a<<"] = ["<<v<<"]"<<endl;
                }

                delete dollarstring;
                return acount;
        }

        int
        argv_parser::args( std::string args, std::string separators )
        {
                if( args.empty() ) return 0;
                strtool::stdstring_tokenizer toker;
                static const int maxargs = 256; // max number of arguments. This size is completely arbitrarily chosen.
                char *cargs[maxargs];

                int count = 0;
                toker.tokenize( args, separators );
                int argbufsize = 256;
                while( toker.has_tokens() && count < maxargs - 1)
                {
                        cargs[count] = new char[argbufsize]; // if this isn't enough... there's a problem.
                        assert( cargs && "memory allocation error!" );
                        strncpy( cargs[count], toker.next_token().c_str(), argbufsize );
                        //        CERR << "parse(): token= [" << cargs[count] << "]" << endl;
                        ++count;
                }
                cargs[count] = 0;
                int ret = count ? this->args( count, cargs, 0, 0 ) : 0;
                for( int i = 0; i < count; i++ ) delete cargs[i];
                return ret;
        }


        const std::string
        argv_parser::dump_help( bool scv /* show current values? */ ) const
        {
                property_store::const_iterator iter = this->helpmap.begin();
                std::string outs;
                std::string key, val, realkey;
                //     outs += "argument_name:";
                //     if( scv ) outs += " [current value]";
                //     outs += "\n\thelp text for argument.\n\n";
                while( iter != this->helpmap.end() )
                {
                        key = (*iter).first;
                        outs += key;
                        outs += ':';
                        if( scv )
                        {
                                val = this->get_string( key ); // this handles dashes for us, (*iter).second does not.
                                if( ! val.empty() )
                                {
                                        outs += " [";
                                        outs += val;
                                        outs += "]";
                                }
                        }
                        outs += "\n\t";
                        outs += this->get_help( key );
                        outs += "\n\n";
                        ++iter;
                }
                outs += "\nAll arguments must start with either - or --, like -help or --help.\n";
                outs += "Arguments and their values may optionally be separated by '=', and a '=' is required if the value starts with a '-'.\n";
                outs += "Order of the arguments is insignificant, and if an arg is repeated only the last one is accepted.\n";
                return outs;
        }

        argv_parser & // static 
        argv_parser::args( int argc, char *argv[] )
        {
                argv_parser::args().args( argc, argv, 0 );
                return argv_parser::args();
        }

        argv_parser & // static 
        argv_parser::args()
        {
//                 return phoenix::phoenix<argv_parser>::instance();
		static argv_parser bob;
		return bob;
        }

        bool
        argv_parser::is_help_set()
        {
                return this->is_set( "help" ) || this->is_set( "-?" ) ;
        }


        bool
        argv_parser::is_set( const std::string &key ) const
        {
                //std::cout << "is_set( "<<key<<")???"<<endl;
                if( property_store::is_set( key ) ) return true;
                if( key.find( "-" ) != 0 ) // non-dashed argument
                {
                        if( property_store::is_set( string("--")+key ) ) return true;
                        if( property_store::is_set( string("-")+key ) ) return true;
                }
                return false;
        }

} // namespace shellish
