
#include <iostream> // cerr

#include <s11n.net/shellish/shellish_debuggering_macros.hpp> // COUT/CERR
#include <s11n.net/shellish/aliaser.hpp>

namespace shellish
{
        aliaser::aliaser(){}
        aliaser::~aliaser(){}

        aliaser::map_type &
        aliaser::map()
        {
                return this->m_map;
        }
        const aliaser::map_type &
        aliaser::map() const
        {
                return this->m_map;
        }

        aliaser &
        aliaser::instance()
        {
                static aliaser meyers;
                return meyers;
        }

        void
        aliaser::alias( const std::string & al, const std::string & expanded )
        {
                //CERR << "alias("<<al<<",["<<expanded<<"])"<<std::endl;
                if( expanded.empty() )
                {
                        aliaser::map_type::iterator it = map().find( al );
                        if( map().end() != it )
                        {
                                map().erase( it );
                        }
                        return;
                }
                map()[al] = expanded;
        }

        std::string
        aliaser::expand( const std::string & input ) const
        {
                if( input.empty() ) return input;
                std::string arg = input.substr( 0, input.find_first_of(" \t\n") );
                aliaser::map_type::const_iterator it = map().find( arg );
                if( map().end() == it )
                {
                        return input;
                }
                std::string code = (*it).second;
                if( code == arg )
                {
                        return arg;
                }
                std::string tmp = code + ( (arg.size() == input.size()) ? std::string() : input.substr( arg.size() ) ); // skip the first token
                //                 CERR << "alias resolution for ["<<input<<"] = ["<<tmp<<"]"<<std::endl;

                // not thread safe:
                static unsigned int loopblocker = 0;
                if( ++loopblocker > 9 )
                {
                        CERR << "******************** WARNING ********************\n" 
                             << "expand() has been through "<<loopblocker
                             << " iterations. That's probably too many, so we're aborting alias expansion. "
                             << "You probably have aliases which expand to each other. Please check this content for possible problems: ["<<input<<"]."
                             << "\nIf you believe this to be a bug, please fix it in " << __FILE__ <<", line " <<__LINE__<<"."
                             << std::endl;
                        loopblocker = 0;
                        return tmp;
                }
                --loopblocker;
                tmp = this->expand( tmp );
                return tmp;
        }


} // namespace shellish
