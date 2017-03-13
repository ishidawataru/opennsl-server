////////////////////////////////////////////////////////////////////////
// A test & demo app for s11n[lite].
// Author: stephan@s11n.net
// License: Do As You Damned Well Please
////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#  undef NDEBUG // we always want assert() to work
#endif

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory> // auto_ptr
#include <stdexcept>


////////////////////////////////////////////////////////////////////////
#include <s11n.net/s11n/s11nlite.hpp> // s11n & s11nlite frameworks
#include <s11n.net/s11n/proxy/std/vector.hpp>
#include <s11n.net/s11n/proxy/std/list.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// misc util stuff
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
#include <s11n.net/s11n/algo.hpp> // free_list_entries()
////////////////////////////////////////////////////////////////////////

typedef std::vector<std::string> StringVec;
#define TEST_WITH_PTR 1
#if TEST_WITH_PTR
    typedef std::vector< StringVec * > StringVecVec;
#else
    typedef std::vector< StringVec > StringVecVec;
#endif
struct AType {
        AType()
        {
                CERR << "AType::AType\n";
                for( int i = 0; i < 5; ++i )
                {
                        this->m_vec.push_back( i * i );
                }
        }
        virtual ~AType()
        {
                ::s11n::cleanup_serializable( this->vecvec );
        }

        virtual bool operator()( s11nlite::node_type & dest ) const
        {
                CERR << "AType::serialize\n";
                typedef s11nlite::node_traits NT;
                NT::class_name( dest, "AType" );
                s11n::serialize_subnode( dest, "vec", this->m_vec );
                s11n::serialize_subnode( dest, "vecvec", this->vecvec );
                return true;
        }

        virtual bool operator()( const s11nlite::node_type & src )
        {
                CERR << "AType::deserialize\n";
                this->m_vec.clear();
                s11n::deserialize_subnode( src, "vec", this->m_vec );
                this->vecvec.clear();
                s11n::deserialize_subnode( src, "vecvec", this->vecvec );
                return true;
        }

        StringVecVec vecvec;

private:
        std::vector<int> m_vec;

};

struct BType : public AType {
        BType()
        {
                CERR << "BType::BType\n";
        }
        virtual ~BType(){}
        virtual bool operator()( s11nlite::node_type & dest ) const
        {
                this->AType::operator()( dest );
                CERR << "BType::serialize\n";
                typedef s11nlite::node_traits NT;
                NT::class_name( dest, "BType" );
                return true;
        }

        virtual bool operator()( const s11nlite::node_type & src )
        {
                this->AType::operator()( src );
                CERR << "BType::deserialize\n";
                return true;
        }

};

struct X : public BType {
        X()
        {
                CERR << "X::X\n";
        }
        virtual ~X(){}

        virtual bool operator()( s11nlite::node_type & dest ) const
        {
                this->BType::operator()( dest );
                CERR << "X::serialize\n";
                typedef s11nlite::node_traits NT;
                NT::class_name( dest, "X" );
                return true;
        }

        virtual bool operator()( const s11nlite::node_type & src )
        {
                this->BType::operator()( src );
                CERR << "X::deserialize\n";
                return true;
        }
};

#define S11N_TYPE AType
#define S11N_TYPE_NAME "AType"
#include <s11n.net/s11n/reg_s11n_traits.hpp>

#define S11N_TYPE BType
#define S11N_TYPE_NAME "BType"
#define S11N_BASE_TYPE AType
#include <s11n.net/s11n/reg_s11n_traits.hpp>

#define S11N_TYPE X
#define S11N_TYPE_NAME "X"
#define S11N_BASE_TYPE AType
#include <s11n.net/s11n/reg_s11n_traits.hpp>

void test_hier();
void test_patsbug();
int
main( int argc, char **argv )
{

        s11nlite::serializer_class( "parens" );
	try
	{
		test_hier();
		//         test_patsbug();
	}
	catch( const std::exception & ex )
	{
		CERR << "EXCEPTION: " << ex.what() << "\n";
		return 1;
	}
        return 0;
}


template <typename LT>
void child_sizes( const LT & list ) {

        typedef typename LT::const_iterator CIT;
        typedef s11n::const_object_reference_wrapper<typename LT::value_type> CW;
        CIT it = list.begin(),
                et = list.end();
        int c = 0;
        CERR << "child_sizes()...\n";
        for( ; et != it; ++it ) {
                CW wr(*it);
                CERR << "Child #"<<std::dec<<c++<<": size() = " << wr().size()<<"\n";
        }

}

void test_patsbug()
{
        typedef std::list<std::string> SList;
#if TEST_WITH_PTR
        typedef std::list< SList * > ListList;
#else
        typedef std::list< SList > ListList;
#endif

        typedef s11n::s11n_traits<SList>::serializable_type FooT;
        CERR << "classname<SList> ="<<s11n::s11n_traits<SList>::class_name(0)<<"\n";

        ListList biggie;
        SList l1;
        SList l2;
        SList l3;


        l1.push_back( "list one" );
        l2.push_back( "list two" );
        l3.push_back( "list three" );

#if TEST_WITH_PTR
#  define DEREF(A) (&A)
#else
#  define DEREF(A) A
#endif
        biggie.push_back( DEREF(l1) );
        biggie.push_back( DEREF(l2) );
        biggie.push_back( DEREF(l3) );
        child_sizes( biggie );

        using namespace s11nlite;

        CERR << "The input lists:\n";
        save( l1, std::cout );
        save( l2, std::cout );
        save( l3, std::cout );

        typedef ListList ToListT;
//         typedef std::list<std::string *>
        ToListT tolist;
        node_type node;
        assert( serialize( node, biggie ) );
        CERR << "biggie, in data node:\n";
        save( node, std::cout );
        assert( s11n_cast( biggie, tolist ) );
        CERR << "after s11n_cast: tolist size="<<tolist.size() << "\n";
        child_sizes( tolist );
        CERR << "ser'd list:\n";
        save( biggie, std::cout );
        CERR << "deser'd list:\n";
        save( tolist, std::cout );
#undef DEREF


}



void test_hier()
{


        std::auto_ptr<AType> c(new X);

#if TEST_WITH_PTR
        StringVec * temp = new StringVec;
        StringVec * temp2 = new StringVec;
        StringVec * temp3 = new StringVec;
        temp->push_back("astring");
	temp->push_back("astring1" );
        temp2->push_back("astring2");
        temp3->push_back("astring3");
#else
        StringVec temp;
        StringVec temp2;
        StringVec temp3;
        temp.push_back("astring");
	temp.push_back("astring1" );
        temp2.push_back("astring2");
        temp3.push_back("astring3");
#endif

        using namespace s11nlite;
        // s11nlite::serializer_class("funxml");
        std::ostringstream os, os2; // simulate a file

        c->vecvec.push_back(temp);
        c->vecvec.push_back(temp2);
        c->vecvec.push_back(temp3);

	CERR << "serialize object 'c' ====>\n";
	save( *c, std::cerr );

        StringVecVec ssv;
        if( 1 )
        {
                s11n_cast( c->vecvec, ssv );
                CERR << "Deser'd vector via s11n_cast():\n";
                save( ssv, std::cerr );
		s11n::cleanup_serializable( ssv );
		CERR << "Post-cleanup ssv size == " << ssv.size() << "\n";
        }

	std::string s1, s2;
	s1 = "a string";
	s11n_cast( s1, s2 );
	save( s2, std::cout );

}

