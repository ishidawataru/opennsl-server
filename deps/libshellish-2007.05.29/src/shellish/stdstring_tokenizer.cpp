////////////////////////////////////////////////////////////////////////
// stdstring_tokenizer.hpp - a Java-like string tokenizer based off
// of string_tokenizer.hpp, but which works with std::string instead
// of (char *).
//
// Author: stephan beal <stephan@s11n.net>
// License: Public Domain
////////////////////////////////////////////////////////////////////////


#include <s11n.net/shellish/stdstring_tokenizer.hpp>

namespace shellish { namespace strtool {
	enum quoteEnum
	{ NO_QUOTE = 0, SINGLE_QUOTE, DOUBLE_QUOTE };
//            todo: implement a map<char,int> to count opening/closing quotes so i can add support
//            for open/close { } without having to add to the enum, add if(), etc.
//            map['\''] = 0. increment on an opener, decrement on a closer.


        stdstring_tokenizer::stdstring_tokenizer()
	{
	}

        stdstring_tokenizer::stdstring_tokenizer( const string & str, const string & separators )
        {
                this->tokenize( str, separators );
        }

	void stdstring_tokenizer::tokenize( const string & str, const string & sep )
	{
		const string::size_type ssize = str.size();
		if ( ssize == 0 )
			return;
		if ( ssize == 1 )
		{
			this->m_list.push( str );	//  += str;
			return;
		}
		if ( string::npos == str.find_first_of( sep ) )
		{
			this->m_list.push( str );	// += str;
			return;
		}

		int quoted = NO_QUOTE;
		char chat;
		string mystr;
		bool addit;
		for ( string::size_type pos = 0; ( pos < ssize ); pos++ )
		{
			chat = str[pos];
			addit = true;
			if ( ( chat == '\"' ) && !quoted )
			{
				quoted = DOUBLE_QUOTE;
				addit = false;
			}
			else if ( ( chat == '\'' ) && !quoted )
			{
				quoted = SINGLE_QUOTE;
				addit = false;
			}
			else if ( ( ( chat == '\"' ) && ( quoted == DOUBLE_QUOTE ) ) || ( ( chat == '\'' ) && ( quoted == SINGLE_QUOTE ) ) )
			{
				quoted = NO_QUOTE;
				addit = false;
			}

			if ( !quoted )
			{
				for ( string::size_type i = 0; i < sep.size(); i++ )
				{
					if ( chat == sep[i] )
					{
						m_list.push( mystr );	// += mystr;
						mystr = string();
						addit = false;
						break;
					}
				}
			}

			if ( addit )
				mystr += chat;
		}
		if ( !mystr.empty() )
			m_list.push( mystr );	// += mystr;
		return;
	}

	bool stdstring_tokenizer::has_tokens()const
	{
		return !this->m_list.empty();
	}
	std::string stdstring_tokenizer::next_token()
	{
		string foo = this->m_list.front();
		this->m_list.pop();
		return foo;
	}

	stdstring_tokenizer::~stdstring_tokenizer()
	{
	}

}}				// namespace 
