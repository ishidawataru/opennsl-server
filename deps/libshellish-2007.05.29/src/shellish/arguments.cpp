#include <s11n.net/shellish/arguments.hpp>

namespace shellish {

        arguments::arguments() {}

        arguments & arguments::operator=( const arguments & rhs )
        {
                if( &rhs == this ) return *this;
                this->m_args = rhs.m_args;
                this->m_str = rhs.m_str;
                return *this;
        }

        arguments::arguments( const arguments & rhs )
        {
                this->m_args = rhs.m_args;
                this->m_str = rhs.m_str;
        }

        arguments::arguments( const std::string & input )
        {
                this->tokenize( input );
        }

        void arguments::push_back( const std::string & s )
        {
                this->m_args.push_back( s );

        }

        void arguments::push_front( const std::string & s )
        {
                this->m_args.push_front( s );
        }
        void arguments::tokenize( const std::string & input )
        {
                // That alias expansion is done HERE is a shameful kludge. :(
                // It properly expands aliases which expand to multiple tokens,
                // which is tedious to achieve if that expansion is done in the
                // shellish input layer.
                this->m_args.clear();
// 		std::string s = input;
//                 s = ::shellish::aliases().expand( s );
		this->m_str = input;
                tokenize_command( this->m_str, this->m_args );
//                 this->expand();
        }

        arguments::~arguments()
        {
        }

//         void arguments::expand()
//         {
//                 args_list::iterator it = this->begin(),
//                         et = this->end();
// // 		size_t count = 0;
//                 for( ; et != it; ++it )
//                 {
//                         SHELLISHDEBUG << "expand()ing token ["<<(*it)<<"]\n";
// // 			if( 0 == count++ )
// // 			{ // do automatic ~ expansion only on first token.
// // 				::shellish::env().expand_tilde_home( (*it) );
// // 			}
//                         (*it) = ::shellish::expand( (*it) );
//                 }
//         }

        const arguments::args_list &
        arguments::argv() const { return this->m_args; }

        arguments::args_list &
        arguments::argv() { return this->m_args; }

        shellish::args_list::const_iterator
        arguments::begin() const
        {
                return this->argv().begin();
        }
        shellish::args_list::iterator
        arguments::begin()
        {
                return this->argv().begin();
        }

        shellish::args_list::const_iterator
        arguments::end() const
        {
                return this->argv().end();
        }
        shellish::args_list::iterator
        arguments::end()
        {
                return this->argv().end();
        }


        size_t
        arguments::argc() const { return this->m_args.size(); }

	bool arguments::empty() const
	{
		return this->m_args.empty();
	}

        const std::string &
        arguments::str() const { return this->m_str; }

        std::string arguments::operator[]( unsigned int n ) const
        {
                if( n >= this->argc() ) return "";
                shellish::args_list::const_iterator cit = this->argv().begin();
                for( unsigned int i = 0; i < n; ++i )
                {
                        ++cit;
                }
                return (*cit);
        }

//         std::string arguments::tostring() const
//         {
//                 shellish::args_list::const_iterator cit = this->argv().begin(),
//                         cet = this->argv().end();
//                 std::string ret;
//                 //for( ; cet != cit; ++cit )
// 		while(true)
//                 {
//                         ret += (*cit);
// 			if( ++cit != cet )
// 			{
// 				ret += " ";
// 			}
// 			else
// 			{
// 				break;
// 			}
//                 }
// //                 stringutil::trim_string( ret );
//                 return ret;
//         }

        std::string
        arguments::shift()
        {
                args_list::iterator it = this->m_args.begin();
                if( this->m_args.end() == it ) return std::string();
                std::string ret = (*it);
                this->m_args.erase( it );
                // strip first token from string part, respecting shell-like quoting:
		const std::string sep = " \t\n";
                ::shellish::strtool::stdstring_tokenizer st( this->m_str, sep );
                if( st.has_tokens() )
                {
                        std::string arg0 = st.next_token();
                        if( arg0.size() < this->m_str.size() )
                        {
				// chop off arg0 and trailing whitespace
                                this->m_str = this->m_str.substr( this->m_str.find_first_not_of( sep, arg0.size() ) );
//                                 stringutil::trim_string( this->m_str );
                        }
                        else this->m_str = "";
                }
                //SHELLISHDEBUG << "shift() = [" << this->m_str << "]" << std::endl;
                return ret;
        }

} // namespace
