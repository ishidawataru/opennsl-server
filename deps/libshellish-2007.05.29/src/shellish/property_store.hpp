// Author: stephan beal <stephan@s11n.net>
// License: Public Domain
#ifndef  shellish_PROPERTYSTORE_HPP_INCLUDED
#define  shellish_PROPERTYSTORE_HPP_INCLUDED 1

#include <iostream>
#include <map>
#include <sstream>
#include <s11n.net/shellish/strtool.hpp>

namespace shellish
{
	/**
           property_store is a class for storing arbitrary key/value
           pairs.

           i often find myself needing classes which contain an
           arbitrary number of arbitrary properties, and this
           interface has worked very well for me.

           It must be strongly stressed that this class is designed
           solely with utility, ease-of-use and code maintenance costs
           in mind. Not one iota of concern has been given to
           optimization! It is not a lightweight object, nor is it
           efficient. Do not use this class when speed is of the
           essence! That said, it should fast enough for almost all
           standard property-list uses. (i often use thousands of them
           as property containers for game pieces.)

           It's get() and set() template functions support
           any types which are both instreamable and outstreamable
           (implements both operator<<(std::ostream) and
           operator>>(std::istream)).

           It follows STL container conventions, so it can be used in
           many container contexts and with many generic algorithms.

           Years later...

           This class is in need of optimization. One thought is to
           internally use a reference-counted string class instead of
           std::string, but i need to do some timings.
        */
	class property_store
	{
	      public:

                /**
                   The map type this object uses to store items
                   internally.
                 */
		typedef std::map < std::string, std::string > map_type;
		typedef map_type::value_type value_type;

                /** For compatibility with std::map */
                typedef std::string key_type; 
                /** For compatibility with std::map */
                typedef std::string mapped_type; 

                /**
                   For iterating over properties using STL
                   conventions.
                 */
		typedef map_type::iterator iterator;

                /**
                   For iterating over properties using STL
                   conventions.
                 */
		typedef map_type::const_iterator const_iterator;

		/**
                   std::string propval = props["bar"] is functionally
                   identical to get_string("bar").  Unlike std::map and
                   the like, calling this operator with a key which is
                   not in the object does not create a new entry - it
                   simply returns an empty string in that case.
                */
		const std::string operator[] ( const std::string & key ) const;

                /**
                   For compatibility with std::map. Inserts a
                   value_type (i.e., pair<string,string>) into this
                   object.  It calls set_string(), so it is compatible
                   with subclasses which do custom handling in that
                   method.
                 */
                void insert( const value_type & );

                property_store();
                virtual ~ property_store();

		/**
                   Returns the number of items in this object. This is
                   a constant-time operation.
                */
		unsigned long count_properties() const;

		/**
                 * Returns the value for key, or defaultVal if the key
                 * is not set. 
                 *
                 * Note to subclassers: This is the
                 * "master" getter, so subclasses which want to
                 * generically alter getXXX() behaviour need only
                 * override this function. Almost all other getXXX()
                 * functions call this one, so do not call them from
                 * inside this function.
                 */
		virtual std::string get_string( const std::string & key, const std::string & defaultVal = std::string() )const;

		/**
                   Sets the given key to the given value.
                   See get_string(): same notes apply here.

                   If key.empty() then this function does nothing.
                */
		virtual void set_string( const std::string & key, const std::string & val );

		/**
                   See set_string(). This function is identical except
                   that it converts val to string before saving it. If
                   this type conversion is not possible it will fail
                   at compile time. A value-conversion failure, on the
                   other hand, is not caught at compile time.
                 */
                template < typename T > void set( const std::string & key, const T & val )
		{
			this->set_string( key, strtool::to( val ) );
		}

		/**
                   The <const char *> variants of get() and set() are
                   to help the developer avoid having to cast so much
                   and to help out compilers which have mediocre
                   template support. :/
                */
		inline void set( const char *key, const char *val )
		{
			this->set_string( std::string( key ), std::string( val ) );
		}
                /**
                   Overloaded for strings-via-streams reaons.
                 */
		inline void set( const std::string & key, const char *val )
		{
			this->set_string( key, std::string( val ) );
		}
                /**
                   Overloaded for strings-via-streams reaons.
                 */
		inline void set( const char *key, const std::string & val )
		{
			this->set_string( key, std::string( val ) );
		}

		/**
                   See get_string(). This function is identical except
                   that the returned string is converted to type T. If
                   this type is not possible it will fail at compile
                   time. A value-conversion failure, on the other
                   hand, is not caught at compile time.
                 */
		template < typename T > T get( const std::string & key, const T & val ) const
		{
			std::string foo = this->get_string( key, strtool::to( val ) );
			return strtool::from<T>( foo, val );
		}

                /**
                   get(): see set(). Same notes apply.
                */
		inline std::string get( const char *key, const char *val ) const
		{
			return this->get_string( std::string( key ), std::string( val ) );
		}
                /**
                   Overloaded for strings-via-streams reasons.
                 */
		inline std::string get( const std::string & key, const char *val ) const
		{
			return this->get_string( key, std::string( val ) );
		}
                /**
                   Overloaded for strings-via-streams reasons.
                 */
		inline std::string get( const char *key, const std::string & val ) const
		{
			return this->get_string( std::string( key ), val );
		}


                /**
                   Returns true if this object contains the given property.
                */
		virtual bool is_set( const std::string & key ) const;

                /**
                   Removes the given property from this object.
                */
		virtual bool unset( const std::string & key );

		/**
                   Removes all entries from this object.
                 */
		virtual bool clear_properties();

                /**
                   Same as clear_properties().
                */
                void clear() { this->clear_properties(); }

		/**
                   Note that the set/getTYPE() variants are mostly to
                   keep old code working. Please use set() and get()
                   when possible. Sometimes it is more convenient to
                   use these instead of get()/set(), especially with
                   something like:
                   
                   <pre>
                   props.set( "foo", false ); // ambiguous: may be bool, const char *, or bool or even int zero :/
                   </pre>

                */
		inline void set_bool( const std::string & key, bool val )
		{
			return this->set( key, val );
		}

		/**
                   get_bool(key) is an exception to the set/getXXX()
                   rule: it returns true if key's value is true, as
                   evaluated by the static function bool_val().
                */
		bool get_bool( const std::string & key, bool defaultVal ) const;



		/**
                 * Returns the bool value of the passed string.
                 * The following string values are considered equal to true:
                 *     true, TRUE, True
                 *     yes, YES, Yes, y, Y
                 *     1
                 * Anything else evaluates to false.
                 * CASE IS IMPORTANT! (Sorry, when i learn how to easily lowercase a c++ string
                 * i'll fix all that.)
                 */
		static bool bool_val( const std::string & key );


		/**
                 * Returns the first item in the data map.
                 * You can use this to iterate, STL-style:
                 * <pre>
                 *   map_type::iterator it = props.begin();
                 *   while( it != props.end() ) { ... ++it; }
                 * </pre>
                 *
                 * Note that the iterator represents a
                 * std::pair<string,string>, so use (*it).first to get
                 * the key and (*it).second to get the value.
                 */
		iterator begin();

                /**
                   Returns a const_iterator pointing at this object's first property.
                */
		const_iterator begin() const;

		/**
                 * The after-the-end iterator for the data map.
                 */
		iterator end();
		/**
                 * The after-the-end iterator for the data map.
                 */
		const_iterator end() const;

		/**
                 * Returns end() if the key is not in our map, otherise it returns
                 * that iterator. Use the iterator's .first member to get the key,
                 * and .second to get the value. However, you SHOULD call get_string( (*iter).first )
                 * to get the value, so subclasses can do some funniness with the key,
                 * like argv_parser does. Thus:
                 * <pre>
                 *   std::string val = "not found";
                 *   iter = foo.find( "key" );
                 *   if( iter == foo.end() ) { return val; }
                 *   val = foo.get_string( (*iter).first );
                 * </pre>
                 *
                 * Such usage helps guaranty polymorphic behaviour.
                 *
                 * todo?: make this the master getter, instead of
                 * get_string(), for purposes of overriding getXXX()
                 * behaviour?
                 */
		virtual map_type::iterator find( const std::string & key );

		/**
                   merge() copies all properties from src to dest. It returns the
                   number of properties copied.

                   This is potentially a very expensive operation.
                */
		static unsigned int merge( const property_store & src, property_store & dest );
		/**
                   Merges all properties from src into this object. Returns the number
                   of properties merged.
                */
		unsigned int merge( const property_store & src );


                /**
                   Returns the number of properties in this object.
                */
                map_type::size_type size() const;

		/**
                 * Returns the internally-used map_type (see the typedefs, above).
                 */
		map_type & get_map();
		/**
                 * Returns the internally-used map_type (see the typedefs, above).
                 */
		const map_type & get_map() const;

		bool empty() const;

	      private:
		map_type m_map;
	};			// class property_store


};				// namespace
#endif //  shellish_PROPERTYSTORE_HPP_INCLUDED
