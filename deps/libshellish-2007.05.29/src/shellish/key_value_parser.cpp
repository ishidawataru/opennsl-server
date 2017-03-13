/**
Author: stephan beal <stephan@s11n.net>
License: Public Domain
*/

#include <iostream>
#include <s11n.net/shellish/key_value_parser.hpp>

namespace shellish
{
	using namespace std;

        std::ostream & operator<<( ostream & os, const key_value_parser & obj )
	{
		os << obj.key(  );
		os << std::string( "=" );	// gcc3 bitches if this is a char *???
		os << obj.value(  );
		return os;
	}

	key_value_parser::key_value_parser(  ):m_key( "" ), m_val( "" ), m_line( "" )
	{
	}

      key_value_parser::key_value_parser( const string & ln ):m_key( "" ), m_val( "" ), m_line( "" )
	{
		parse( ln );
	}

	bool key_value_parser::parse( const string & ln, const string & delim )
	{
		m_line = ln;
		m_key = "";
		m_val = "";

		string::size_type offset = m_line.find( delim );

		if ( string::npos == offset  )
                {
                        return false;
                }
                m_key = m_line.substr( 0, offset );
                m_val = m_line.substr( offset + delim.length(  ) );

		// strip leading/trailing spaces from m_key and m_val.
		// there must be a simpler (or at least more graceful) way...
		static const std::string space( " \t" );
		while ( !m_key.empty(  ) && m_key.find_last_of( space ) == ( m_key.size(  ) - 1 ) )
		{		// trailing key whitespace
			m_key.erase( m_key.size(  ) - 1 );
		}
		while ( !m_key.empty(  ) && ( m_key.find_first_of( space ) == 0 ) )
		{		// leading key whitespace
			m_key.erase( 0, 1 );
		}

		while ( !m_val.empty(  ) && ( m_val.find_first_of( space ) == 0 ) )
		{		// leading val whitespace
			m_val.erase( 0, 1 );
		}
		while ( !m_val.empty(  ) && m_val.find_last_of( space ) == ( m_val.size(  ) - 1 ) )
		{		// trailing val whitespace
			m_val.erase( m_val.size(  ) - 1 );
		}
		// Whew. Blessed indeed is Perl.
		//CERR << "m_key=["<<m_key<<"] m_val=["<<m_val<<"]"<<endl;
		return m_key.size(  ) > 0;
	}


} // namespace 
