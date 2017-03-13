#include <sstream>
#include <iostream>

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/s11n_debuggering_macros.hpp>


class Base {

protected:
	Base() : common(4) {}
public:
	virtual ~Base() {}
	int common;
	virtual int go() = 0;

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		typedef s11nlite::node_traits NT;
		NT::class_name( dest, "Base" );
		NT::set( dest, "common", common );
		return true;
	}

	virtual bool operator()( const s11nlite::node_type & src )
	{
		typedef s11nlite::node_traits TR;
		common = TR::get( src, "common", int(0) );
		return true;
	}
};

class One : public Base {

protected:
	int num;

public:
	One() : num(1) {}
	virtual ~One() {}
	int go() { return num; }

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		if (! this->Base::operator()( dest ) ) return false ;
		typedef s11nlite::node_traits TR;
		TR::class_name( dest, "One" );
		TR::set( dest, "num", num );
		return true;
	}

	virtual bool operator()( const s11nlite::node_type & src )
	{
		this->Base::operator()( src );
		typedef s11nlite::node_traits TR;
		num = TR::get( src, "num", int(0) );
		return true;
	}
};


class Two : public Base {

public:
	Two() {}
	virtual ~Two() {}
	int go() { return 2; }

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		this->Base::operator()( dest );
		typedef s11nlite::node_traits TR;
		TR::class_name( dest, "Two" );
		return true;
	}

	virtual bool operator()( const s11nlite::node_type & src )
	{
		this->Base::operator()( src );
		typedef s11nlite::node_traits TR;
		return true;
	}
};

class Final {
public:
	Base *base;
	Final(Base *base) : base(base) {}
	Final() {}
	virtual ~Final() {}

	virtual bool operator()( s11nlite::node_type & dest ) const
	{
		typedef s11nlite::node_traits TR;
		TR::class_name( dest, "Final" );
		s11nlite::serialize_subnode( dest, "base", *base );
		return true;
	}

	virtual bool operator()( const s11nlite::node_type & src )
	{
		typedef s11nlite::node_traits TR;

		// How do I deserialize base? the following line segfaults :(:(
		// s11nlite::deserialize_subnode( src, "base", *base );
		const s11nlite::node_type * ch = s11n::find_child_by_name( src, "base" );
		if( ! ch )
		{
			CERR << "Deser of 'base' member failed: node not found!\n";
			return false;
		}
		base = s11nlite::deserialize<Base>( *ch );
		return 0 != base;
	}
};

#define S11N_TYPE Final
#define S11N_TYPE_NAME "Final"
#include <s11n.net/s11n/reg_s11n_traits.hpp>

#define S11N_TYPE Base
#define S11N_TYPE_NAME "Base"
#define S11N_ABSTRACT_BASE
#include <s11n.net/s11n/reg_s11n_traits.hpp>

#define S11N_TYPE One
#define S11N_TYPE_NAME "One"
#define S11N_BASE_TYPE Base
#include <s11n.net/s11n/reg_s11n_traits.hpp>

#define S11N_TYPE Two
#define S11N_TYPE_NAME "Two"
#define S11N_BASE_TYPE Base
#include <s11n.net/s11n/reg_s11n_traits.hpp>


#include <s11n.net/s11n/type_traits.hpp>
template <typename T>
void try_traits( const std::string & tname )
{
	typedef ::s11n::type_traits<T> TT;
	CERR << "try_traits("<<tname<<"): is_pointer_type == " << TT::is_pointer_type
	     << ", is_const_type == " << TT::is_const_type
	     << ", is_reference_type == " << TT::is_reference_type
	     << "\n";
}

int
main(int argc, char **argv) {

	One member;
	// Two member;
	Final mike(&member);

	Base * x = s11n::cl::classload<Base>("One");
	std::cout << "x=="<<std::hex<<x<<'\n';
	delete x;
	x = s11n::tr::s_factory<Base>()("One");
	std::cout << "x=="<<std::hex<<x<<'\n';
	delete x; x = 0;

	std::cout << "Before, go() gives: " << mike.base->go() << std::endl;

	std::ostringstream out;
	s11nlite::save( mike, out );

	CERR << "Serialized, looks like:\n" << out.str() << "\n";

	std::istringstream in(out.str());

	Final *final = s11nlite::load_serializable<Final>( in );

	CERR << "final == "<< std::hex << final << "\n";
	if(final) {
		CERR << "class_name() of deser'd object = " << s11n::s11n_traits<Final>::class_name(final) << "\n";

		std::cout << "After deserialization, go says: " << final->base->go() <<
			std::endl;
		s11nlite::save( *final, std::cout );
	}
	else {
		std::cerr <<"oops, didn't work" << std::endl;
		return 1;
	}
	delete final;
	final = 0;

	CERR << "Some s11n::type_traits<> tests...\n";
#define TTR(T) try_traits< T >(# T);
	TTR(int);
	TTR(int *);
	TTR(int &);
	TTR(const double &);
	TTR(const int *);
#undef TTR

}
