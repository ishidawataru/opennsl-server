#include <s11n.net/shellish/shellish.hpp>
#include <s11n.net/shellish/shellish_debuggering_macros.hpp>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <s11n.net/shellish/path_finder.hpp>

namespace shellish {

        /**
           loads an shellish::ui::menu from the file args[1].

           File format:

           Menu Entry Label|cmd to pass to shellish::dispatch()

           OR:

           cmd to pass to dispatch()


           Lines starting with ';' or '#' are treated as
           comment lines, and are skipped, as are empty lines.
        */
        int
        handle_emenu( const arguments & args )
        {
                std::ostream & os = ::shellish::ostream();
                if( 1 == args.argc() )
                {
                        os << "Usage error: $1 must be an emenu file name." << std::endl;
                        return ErrorUsageError;
                }
                typedef path_finder PF;
                static PF path;
                static bool donethat = false;
                if( (!donethat) && (donethat=true) )
                {
                        std::string dflt = std::string(".:")
                                + shellish_HOME_LIB_DIR + "/menu:"
                                + shellish_SHARED_DATA_DIR + "/menu:";
                        env().expand_vars_inline(dflt);
                        path.add_path( dflt );
                        path.add_extension( ".emenu" );
                }
                std::string menuname = args[1];
                std::string mfilename = path.find( menuname );
                if( mfilename.empty() )
                {
                        os << "Error: no menu file found for menu '"
                           <<menuname<<"'. path=["
                           <<path.path_string()<<"]"<<std::endl;
                        return ErrorResourceNotFound;
                }
                std::ifstream mfile( mfilename.c_str() );
                if( ! mfile )
                {
                        os << "Error opening menu file '"<< mfilename<<"'!"<< std::endl;
                        return ErrorResourceAcquisition;
                }
                ::shellish::ui::menu menu("emenu: " + mfilename);
                std::string line;
                size_t linenum = 0;
                std::string lbl, cmd;
                while( ! std::getline( mfile, line ).eof() )
                {
                        ++linenum;
                        if( line.empty() || ::shellish::is_comment_line(line) ) continue;
                        ::shellish::strtool::stdstring_tokenizer tok(line, "|");
                        try
                        {
                                lbl = tok.next_token();
                                if( tok.has_tokens() )
                                {
                                        cmd = tok.next_token();
                                }
                                else
                                {
                                        cmd = lbl;
                                }
                                menu.add( lbl, ::shellish::dispatch, ::shellish::arguments(cmd) );
                        }
                        catch(...)
                        {
                                os << mfilename << ":"<<linenum << ": line format error in line:\n"
                                   << line << ".\n";
                        }
                }
                return process_menu( menu, true );
        }

        namespace test {

                int menu_one( const shellish::arguments & args )
                {
                        std::ostream & os = ::shellish::ostream();
                        os << "menu_one(" << args.str() << ")\n";
                        return 0;
                }

                int menu_two( const shellish::arguments & args )
                {
                        std::ostream & os = ::shellish::ostream();
                        os << "menu_two(" << args.str() << ")\n";
                        return 42;
                }


                int test_emenu( const shellish::arguments & args )
                {
                        typedef shellish::ui::menu_entry ME;
                        typedef shellish::ui::menu M;
                        typedef shellish::arguments ARGS;
                        
                        M menu( "Bob's Menu Botique" );
                        menu.add( "Action One", menu_one );
                        menu.add( "Action Two", menu_two );
                        menu.add( "Action Foo", shellish::dispatch, ARGS("! ls") );
                        menu.add( "Action Foo v. 2", shellish::dispatch, ARGS("! ls -la") );
                
                        int ret = shellish::ui::process_menu( menu, true );
                        CERR << "Running menu again, this time without repeat:\n";
                        ret = shellish::ui::process_menu( menu, false );
                        return ret;
                }

        } // namespace test

        void init_shellish_ui()
        {
                map_commander( "emenu", handle_emenu, "Loads and processes an shellish 'menu' file." );
                map_commander( "emenu-test", test::test_emenu, "Runs an emenu test." );
        }

        static int bogus_init_var = (init_shellish_ui(),0);



        /**
           Internal comparer type to compare commander_menu entries
           for same-namedness.
        */
        struct menu_item_name_compare
        {
                explicit menu_item_name_compare( const std::string name ) : m_name(name) {}
                template <typename MenuEntry>
                bool operator()( const MenuEntry & rhs ) const
                {
                        return this->m_name == rhs.name;
                }
        private:
                std::string m_name;
        };


        // This func is way too long...
        int ui::process_menu( const menu & themenu,
                              bool loop )
        {
                std::ostream & os = ::shellish::ostream();
                typedef menu::menu_entry_list LT;
                const LT & mlist = themenu.entries;
                menu::menu_entry_list::const_iterator cit = mlist.begin(),
                        cet = mlist.end();
                typedef std::map<int,std::string> Num2MenuMap;
                Num2MenuMap n2m;
                Num2MenuMap::iterator nit, net;
                int num = 0;
                const int exitat = num++;
                static const std::string cancelstr = "Cancel";
                n2m[exitat] = cancelstr;
                for( ; cet != cit; ++cit )
                {
                        n2m[num++] = (*cit).name;
                }
                std::string line;
                int ret = 0;
                bool render = true;
                ::shellish::arguments inargs;
                ::shellish::arguments outargs;
                do
                {
                        if( render )
                        {
                                os << (themenu.name.empty()
                                       ? "Menu:" 
                                       : themenu.name )
                                   << std::endl;
                                nit = n2m.begin();
                                net = n2m.end();
                                for( ; net != nit; ++nit )
                                {
                                        os << "\t"<<(*nit).first << ":\t"<< (*nit).second << "\n";
                                }
                        }
                        render = false;
                        line = shellish::prompt_for_input( "Enter menu entry number: " );
                        if( cancelstr==line )
                        {
                                // if( line.empty() ) os << std::endl; // terminal cleanup :/
                                ret = 0;
                                break;
                        }
                        if( line.empty() ) continue;
                        inargs = line;
                        num = strtool::from<int>( inargs.shift(), -1 );
                        if( exitat == num )
                        {
                                ret = 0;
                                break;
                        }
                        if( -1 == num || (net == (nit = n2m.find( num )) ) )
                        {
                                cit  = std::find_if( mlist.begin(),
                                                     mlist.end(),
                                                     menu_item_name_compare(line) );
                                if ( cet == cit )
                                {
                                        os << "Error: '"<<line<<"' is not a menu option." << std::endl;
                                        ret = shellish::ErrorResourceNotFound;
                                        continue;
                                }
                        }
                        else
                        {
                                cit  = std::find_if( mlist.begin(),
                                                     mlist.end(),
                                                     menu_item_name_compare((*nit).second) );
                                if ( cet == cit )
                                {
                                        os << "Internal Error: menu mapping screw-up!" << std::endl;
                                        ret = shellish::ErrorWTF;
                                        break;
                                }
                        }
                        render = true;
                        outargs = (*cit).func_args;
                        if( 0 < inargs.argc() )
                        {
                                ::shellish::args_list::const_iterator init = inargs.begin();
                                for( ; inargs.end() != init; ++init )
                                {
                                        outargs.argv().push_back( (*init) );
                                }
                        }
                        //CERR << "args=["<<outargs.tostring()<<"]\n";
//                         outargs.expand();
                        //CERR << "expand()ed args=["<<outargs.tostring()<<"]\n";
                        ret = ((*cit).func)( outargs );
                        if( 0 != ret )
                        {
                                os << "[error code: " << std::dec << ret << "]" << std::endl;
                        }

                } while( loop );
                return ret;
                
        }

} // namespace shellish
