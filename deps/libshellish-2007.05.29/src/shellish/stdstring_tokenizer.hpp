// Author: stephan beal <stephan@s11n.net>
// License: Public Domain
#ifndef shellish_STDSTRINGTOKENIZER_HPP_INCLUDED
#define shellish_STDSTRINGTOKENIZER_HPP_INCLUDED 1

#include <string>
#include <queue>

namespace shellish { namespace strtool {
	using std::string;
	/**
           stdstring_tokenizer:

           License: Public Domain

           Author: stephan@s11n.net
           
           Based heavily off of work by:
           
           Martin Jones (mjones@kde.org), Torben Weis (weis@kde.org)
           and Waldo Bastian (bastian@kde.org)

           which i originally found as string_tokenizer in the KDE 1.x
           source tree. i have received explicit permission from each
           of those gentlemen to release the string_tokenizer code into
           into the Public Domain. (Many thanks to them for that
           permission!)

	   This class is meant to be API- and behaviour-compatible
	   with string_tokenizer. This implementation is, however,
	   MUCH less efficient, and works on std::strings instead of
	   C-style strings (const char *).
           
           stdstring_tokenizer tokenizes strings in a way which is
           consistent with the way a Unix shell does. This makes it
           appropriate for use in parsing many types of arbitrary user
           input, from command-line arguments to comma-separated
           files.
        */
	class stdstring_tokenizer
	{
	      public:
		stdstring_tokenizer();
                /**
                   Same as creating a stdstring_tokenizer and calling it's tokenize( str, separators ).
                 */
		stdstring_tokenizer( const string & str, const string & separators );
		~stdstring_tokenizer();

		/**
                   str is split up at points matching any element in
                   separators. Adjecent separators in str are
                   interpreted as empty elements. Thus the string
                   "1;;3", separated by ";", has 3 tokens:
                   ("1","","3").

                   To collect the tokens, do this:

<pre>
stdstring_tokenizer tok( "some string", " " );
while( tok.has_tokens() ) cout << "Token: " << tok.next_token() << endl;
</pre>
                 */
		void tokenize( const string & str, const string & separators );
		/**
                   Returns the next token in our list.
                   Calling next_token() when has_tokens() returns
                   false has undefined behaviour.
                 */
		string next_token();
		/**
                   Returns true if this object has more tokens to give you.
                */
		bool has_tokens() const;

	      private:
		typedef std::queue < std::string > queue_type;
		queue_type m_list;
	};

} }				// namespace
#endif // shellish_STDSTRINGTOKENIZER_HPP_INCLUDED
