// Author: stephan beal <stephan@s11n.net>
// License: Public Domain
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include <s11n.net/shellish/property_store.hpp>

namespace shellish
{				// behold the namespace...

	using namespace std;


	const std::string property_store::operator[] ( const string & key ) const
	{
		return this->get_string( key );
	}


	property_store::property_store()
	{
		return;
	}

	property_store::~property_store()
	{
	}

	unsigned long property_store::count_properties() const
	{
		return this->m_map.size();
	}

        void
        property_store::insert( const value_type & v )
        {
                this->set_string( v.first, v.second );
        }

	std::string property_store::get_string( const std::string & key, const std::string & defaultVal ) const
	{
		if ( !this->is_set( key ) )
			return defaultVal;
		// DO NOT call LIBE_{DEBUG,VERBOSE} from here!
		map_type::const_iterator citer = m_map.find( key );
		if ( citer != m_map.end() )
			return ( *citer ).second;
		return defaultVal;
	}

	void property_store::set_string( const std::string & key, const std::string & val )
	{
		if ( key.empty() )
			return;
		//COUT << "set_string("<<key<<" =["<<val<<"]" << std::endl;
		m_map[key] = val;
		return;
	}

	bool property_store::is_set( const std::string & mkey ) const
	{
		std::string key = mkey;	// experiment to try to work around a segfault.
		if ( key.empty() )
			return false;
		map_type::const_iterator iter;
		iter = this->m_map.find( key );
		return ( iter != m_map.end() ) ? true : false;
	}
	bool property_store::unset( const std::string & key )
	{
		map_type::iterator iter;
		iter = m_map.find( key );
		if ( iter == m_map.end() )
			return false;
		m_map.erase( iter );
		return true;
	}

	bool property_store::clear_properties()
	{
		if ( m_map.empty() )
			return false;
		m_map.erase( m_map.begin(), m_map.end() );
		return true;
	}


	property_store::iterator property_store::begin()
	{
		return this->m_map.begin();
	}

	property_store::const_iterator property_store::begin()const
	{
		return this->m_map.begin();
	}

	property_store::iterator property_store::end()
	{
		return this->m_map.end();
	}

	property_store::const_iterator property_store::end()const
	{
		return this->m_map.end();
	}

	property_store::iterator property_store::find( const std::string & key )
	{
		return m_map.find( key );
	}

	property_store::map_type & property_store::get_map()
	{
		return m_map;
	}
	const property_store::map_type & property_store::get_map() const
	{
		return m_map;
	}





        property_store::map_type::size_type
        property_store::size() const
        {
                return this->m_map.size();
        }


	unsigned int
        property_store::merge( const property_store & src, property_store & dest )
	{
		return dest.merge( src );
	}

	unsigned int property_store::merge( const property_store & src )
	{
		if ( &src == this )
		{
			//CERR << "merge() source and destinaion property_store objects are the same! Ignoring!" << endl;
			return 0;
		}
		if ( src.count_properties() == 0 ) return 0;
		const_iterator iter = src.begin();
		std::string key;
		std::string val;
		unsigned int count = 0;
		while ( iter != src.end() )
		{
			++count;
			key = ( *iter ).first;
			this->set_string( key, src.get_string( key, ( *iter ).second ) );
			++iter;
		}
		return count;
	}



	bool property_store::get_bool( const std::string & key, bool defaultVal ) const
	{
		if ( !this->is_set( key ) )
			return defaultVal;
		return property_store::bool_val( this->get_string( key, "???" ) );
	}

	bool property_store::empty() const
	{
		return this->m_map.empty();
	}

        /** phoenix<> initializer functor. */
        struct truesmap_initializer
        {
                typedef std::map<std::string,bool> map_type;
                void operator()( map_type & map )
                {
			map["true"] = true;
			map["TRUE"] = true;
			map["True"] = true;
			map["yes"] = true;
			map["YES"] = true;
			map["Yes"] = true;
			map["y"] = true;
			map["Y"] = true;
			map["1"] = true;
                }
        };

	bool // static 
        property_store::bool_val( const std::string & key )
	{
		typedef std::map < std::string, bool > TrueMap;
//                 typedef phoenix::phoenix<TrueMap,property_store,truesmap_initializer> PHX;
//                 TrueMap & trues = PHX::instance();
		static TrueMap trues;
		if( trues.empty() )
		{
			truesmap_initializer()( trues );
		}
		return trues.end() != trues.find( key );
	}

}
