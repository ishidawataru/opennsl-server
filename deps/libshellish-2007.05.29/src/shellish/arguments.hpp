#ifndef shellish_ARGUMENTS_HPP_INCLUDED
#define shellish_ARGUMENTS_HPP_INCLUDED 1
#include <string>
#include <list>
#include <s11n.net/shellish/stdstring_tokenizer.hpp>

namespace shellish {

        /**
           Helper for tokenizing a command into a list of arguments.
           tgt is emptied before parsing begins.

	   Does no form of var/alias/tilde expansion.

           Returns the number of tokens parsed.

	   StringContainer must support push_back(std::string) and
	   size().
         */
        template <typename StringContainer>
        size_t tokenize_command( const std::string &seq, StringContainer & tgt )
        {
                if( ! tgt.empty() ) tgt.erase( tgt.begin(), tgt.end() );
                ::shellish::strtool::stdstring_tokenizer toker;
                toker.tokenize( seq.c_str(), " \t" );
		std::string tok;
                while( toker.has_tokens() )
                {
			tok = toker.next_token();
                        tgt.push_back( tok );
                }
                return tgt.size();
        }


        /**
           A type representing a list of tokenized input arguments.
         */
        typedef std::list<std::string> args_list;
        // ^^^^ achtung: when this is std::vector it can cause a crash
        // in arguments' copy ctor.

        /**
           arguments, modelled after C's conventional argc/argv,
           represents a set of command-line-style user input
           arguments.

           This type is used for passing user input to
           command handlers.

           $VAR expansion is done as early as possibly by this object.
           That is, when creating one, any arguments tokenized out
           are immediately expanded using shellish::expand().
        */
        class arguments
        {
        public:
                /**
                   Type used to hold a list of argument tokens.
                */
                typedef shellish::args_list args_list;

                /**
                   Sets the objects arguments to the given string and
                   parses them as if they had come in via the shell.
                */
                arguments( const std::string & args );

                /**
                   Copies rhs's data.
                */
                arguments( const arguments & rhs );

                typedef args_list::const_iterator const_iterator;
                typedef args_list::iterator iterator;

                /**
                   Does nothing. arguments created with this
                   constructor are useless except as the target
                   of an assignment.
                 */
                arguments();

                ~arguments();

                /** Equivalent to argv().begin(). */
                const_iterator begin() const;

                /** Equivalent to argv().end(). */
                const_iterator end() const;

                /** Equivalent to argv().begin(). */
                iterator begin();

                /** Equivalent to argv().end(). */
                iterator end();


                /**
                   Copies rhs's data to this object.
                */
                arguments & operator=( const arguments & rhs );

                /**
                   The immutable tokenized list of arguments.
                 */
                const args_list & argv() const;

                /**
                   The mutable list of tokenized arguments.
                */
                args_list & argv();

                /**
                   Returns the number of argument tokens.
                */
                size_t argc() const;


		/**
		   Equivalent to (0 == argc()), but might be implemented
		   in constant time, whereas argc() is probably linear.
		*/
		bool empty() const;

                /**
                   Pops the first argument off of the list and returns it.
                   Returns an empty string if there are no arguments.

                   Any iterators pointing to argv() are invalidated when
                   this method is called.
                 */
                std::string shift();

                /**
                   Returns the "raw" (untokenized) arguments.
                */
                const std::string & str() const;

//                 /**
//                    Returns all args as a string, minus any which
//                    have been shift()ed out.

//                    Avoid using this, due to quoting complications.
//                 */
//                 std::string tostring() const;

                /**
                   Returns the Nth argument in this list, or an empty
                   string if no such argument exists.

                   Note that is technically possible for an empty
                   argument to exist, so do not rely on the fact that
                   an empty argument is the last argument in the list.
                */
                std::string operator[]( unsigned int ) const;

                /**
                   For compatibility with std::list algos.
                */
                void push_back( const std::string & );

                /**
                   For compatibility with std::list algos.
                */
                void push_front( const std::string & );

        private:
//                 /**
//                    Calls shellish::expand() on each token in this object.
//                 */
//                 void expand();


                args_list m_args;
                std::string m_str;
		/**
		   Populates this object from the input string.
		 */
                void tokenize( const std::string & );
        };

	/**
	   Copies [src.begin(),src.end()) to dest.  ListT must be a
	   list-like type contain a string-compatible type.
	*/
	template <typename ListT>
	void copy_to_args( const ListT & src, arguments & dest )
	{
		typedef typename ListT::const_iterator CIT;
		CIT sb = src.begin();
		CIT se = src.end();
		for( ; se != sb; ++sb )
		{
			dest.push_back( (*sb) );
		}
	}

} // namespace

#endif // shellish_ARGUMENTS_HPP_INCLUDED
