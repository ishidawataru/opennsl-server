////////////////////////////////////////////////////////////
// A demo of s11nlite, written for the Software Developer's
// Journal (thus the minimalism!).
// Required linker arguments: -rdynamic -ls11n
#include <s11n.net/s11n/s11nlite.hpp> // s11nlite framework
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR macro
#include <s11n.net/s11n/micro_api.hpp> // minimalistic s11n API
#include <s11n.net/s11n/proxy/std/map.hpp> // a proxy for std::map
#include <s11n.net/s11n/proxy/std/list.hpp> // a proxy for std::list
#include <s11n.net/s11n/proxy/pod/int.hpp> // ... int
#include <s11n.net/s11n/proxy/pod/string.hpp> // ... std::string

// Demonstration of a polymorphic Serializable with an "intrusive" API
struct IntrusivePolymorph
{
private:
    typedef std::map<int,std::string> MapType;
    int m_c;
    MapType m_map;
    static int counter;
public:
    IntrusivePolymorph()
    {
        this->m_c = ++counter;
        this->m_map[0] = "zero";
        this->m_map[1] = "one";
        this->m_map[2] = "two";
    }

    virtual ~IntrusivePolymorph() {}

    // serialize operator:
    virtual bool operator()( s11nlite::node_type & dest ) const
    {
        typedef s11nlite::node_traits NTR;
        NTR::class_name( dest, "IntrusivePolymorph" );
        NTR::set( dest, "number", this->m_c );
        return s11n::serialize_subnode( dest, "map", this->m_map );
    }

    // deserialize operator:
    virtual bool operator()( const s11nlite::node_type & src )
    {
        typedef s11nlite::node_traits NTR;
        this->m_c = NTR::get( src, "number", -1 );
        this->m_map.clear(); // ensure a clean state
        return (-1 != this->m_c) 
            &&
            s11n::deserialize_subnode( src, "map", this->m_map );
    }
};
int IntrusivePolymorph::counter = 0;

// Register with s11n using a "supermacro":
#define S11N_TYPE IntrusivePolymorph
#define S11N_TYPE_NAME "IntrusivePolymorph"
//#define S11N_SERIALIZE_FUNCTOR ... /* not needed in this case */
#include <s11n.net/s11n/reg_s11n_traits.hpp>

int
main( int argc, char **argv )
{
        s11nlite::serializer_class( "funtxt" );
    try
    {
        using namespace s11n::debug;
        trace_mask( trace_mask() | TRACE_CLEANUP );
        typedef std::list<IntrusivePolymorph *> IPList;
        IPList ipl;
        for( int i = 0; i < 5; ++i )
        {
            ipl.push_back( new IntrusivePolymorph );
        }
        s11nlite::micro_api<IPList> micro;
        micro.buffer( ipl ); // save it to a string buffer
        s11n::cleanup_serializable<IPList>( ipl ); // RTFM
        s11n::cleanup_ptr<IPList> delist( micro.load_buffer() );
        s11nlite::save( *delist, std::cout );
    }
    catch( const std::exception & ex )
    {
        CERR << "EXCEPTION: " << ex.what() << "\n";
        return 1;
    }
        return 0;
}
