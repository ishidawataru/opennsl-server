////////////////////////////////////////////////////////////
// Required linker arguments: -rdynamic -ls11n
#include <memory> // auto_ptr
#include <algorithm> // find()
#include <s11n.net/s11n/s11nlite.hpp> // s11nlite framework

#include <s11n.net/s11n/proxy/std/list.hpp> // install list proxy
#include <s11n.net/s11n/proxy/std/map.hpp> // install map proxy
#include <s11n.net/s11n/proxy/pod/int.hpp> // install int proxy
#include <s11n.net/s11n/proxy/pod/string.hpp> // install std::string proxy


#include "widgets.hpp"
#include "widgets_s11n.hpp"

my::Widget::~Widget()
{
	this->nuke_children();
}

void my::Widget::remove_child( Widget * p )
{
	WidgetList::iterator it = std::find( this->children().begin(),
					     this->children().end(),
					     p );
	if( this->children().end() == it ) return;
	p->m_parent = 0;
	this->children().erase( it );
}

void my::Widget::add_child( Widget * p )
{
	WidgetList::iterator it = std::find( this->children().begin(),
					     this->children().end(),
					     p );
	if( this->children().end() != it ) return;
	this->children().push_back( p );
	p->m_parent = this;
}

void my::Widget::parent( Widget * p )
{
	if (p == this->m_parent) return;
	if( this->m_parent && p )
	{
		this->m_parent->remove_child( this );
	}
	if( (this->m_parent = p) )
	{
		this->m_parent->add_child( this );
	}
}

void my::Widget::nuke_children()
{
	if( this->children().empty() ) return;
	WidgetList cp = this->children();
	WidgetList::iterator it = cp.begin();
	WidgetList::iterator et = cp.end();
	this->children().clear();
	for( ; et != it; ++it )
	{
		(*it)->m_parent = 0; // don't use parent(0) because that just causes unneeded calls.
		delete *it;
	}
}

/************************************************************************
SharedData_s11n is a Serializable which de/serializes some shared<T>()
objects.

If you are a template metaprograming wizard, you could theoretically use
typelists to define which shared objects should be serialized.
*/
struct SharedData_s11n
{

	/** Serialize operator. */
	template <typename NodeType>
	bool operator()( NodeType & dest ) const
	{
		typedef ::s11n::node_traits<NodeType> TR;
		TR::class_name( dest, "SharedData" );
		return true;
	}

	/** Deserialize operator. */
	template <typename NodeType>
	bool operator()( const NodeType & src ) const
	{
		return true;
	}

};


void setup_widgets()
{

}

int
main( int argc, char **argv )
{
   std::string format = (argc>1) ? argv[1] : "parens";
   s11nlite::serializer_class( format ); // define output format
   try
   {
	   std::cout << "Hi"<<std::endl;
   }
   catch( const std::exception & ex )
   {
      std::cerr << "EXCEPTION: " << ex.what() << "\n";
      return 1;
   }
   return 0;
}
