// Author: stephan beal <stephan@s11n.net>
// License: Public Domain
#ifndef shellish_KEYVALUEPARSER_H
#define shellish_KEYVALUEPARSER_H

#include <string>
#include <map>
#include <list>

namespace shellish
{
	using namespace std;

	/**
           key_value_parser is a class for parsing "key=value"-style lines,
           like those which would come from a configuration file.
        */
	class key_value_parser
	{
	      public:
                /**
                   Creates a new key_value_parser and runs parse( line ).
                */
		explicit key_value_parser( const string & line );
                /**
                   Creates an empty key_value_parser.
                */
                key_value_parser();
                virtual ~key_value_parser(){}

		/**
                   Parses 'line' into a key/value pair. To be parseable the line must be in the form:

                   key=value

                   Extra whitespace around the '=' removed, as are leading and
                   trailing whitespace around the key and value. This behaviour is
                   arguable but probably desireable in most cases (it is in all of
                   mine, and i wrote the damned thing ;).

                   todo: add a whitespace removal policy as optional 3rd argument?

                   delimiter is the string which separates the key and
                   value, so a line in the format:
    
                       key{alternateDelimiter}value...
    
                   (minus the braces) is parseable. alternateDelimiter can be a set
                   of possible delimiters, such as " \t".

                   That is:
                   parse( "one;two",";" )
                   results in key() == "one" and value() == "two"

                   This function returns false if it does not consider the line to be parseable.
                   Use key() and value() to get the parsed values. Use line() to get the whole
                   string passed to parse (as if you'd ever need it, though subclasses might).
                   line() /is/ guaranteed to be set to line by this call, unlike key() and value().
       
                   If this function returns false, the values returned by key() and value()
                   cannot be considered reliable (i.e., they are undefined).

                   This function will return false if a line contains
                   no key (like '=value'), but empty values are not an
                   error (i.e., they will not cause this function to
                   return false). BUG: in some cases (when delimiter
                   is a space) a no-value key can fail to parse.
                */
		bool parse( const string & line, const string & delimiter = "=" );


		/**
                   Returns the parsed-out key. Only valid if parse() returned true.
                */
		inline const string & key() const
		{
			return this->m_key;
		}

                /**
                   Sets this object's key().
                */
		inline void key( const string & v )
		{
			this->m_key = v;
		}
		/**
                   Returns the parsed-out value (may be empty). Only valid if parse() returned true.
                */
		inline const string & value() const
		{
			return this->m_val;
		}
                /**
                   Sets this object's value().
                */
		inline void value( const string & v )
		{
			this->m_val = v;
		}
		/**
                   Returns the last whole line passed to parse().
                */
		inline const string & line() const
		{
			return this->m_line;
		}

	      private:
		string m_key;
		string m_val;
		string m_line;
		string::const_iterator strIt;
	};

        /**
           enters k.key()=k.value() into os.
        */
        std::ostream & operator <<( std::ostream & os, const key_value_parser & );

};				// namespace
#endif // shellish_KEYVALUEPARSER_H
