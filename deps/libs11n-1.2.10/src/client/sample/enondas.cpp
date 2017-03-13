/**
   Demonstration client app for s11nlite.

   Generates player characters for the Enondas roleplaying system.
   Enondas is similar enough to Tunnels & Trolls that this app can
   be used for that game as well.

   Demonstrates the following libs11n capabilities:

   - Serializing app-wide state.

   - A serializable client-side class.

   - Serialization of rather complex container combinations,
     e.g. map<string,map<string,double>>


Sample usage:

./enondas -d enondas.s11n -s compact -name Bob -kindred dwarf -dice 4 -droplow


Args:

-d DATAFILE (default=enondas.s11n)

-s SERIALIZER (for s11nlite)

-name "PC's NAME"

-kindred pc_race (must be defined in DATAFILE)

-dice X (number of dice to roll for attributes)

-droplow (if set, lowest attribute die is dropped).

-ds ("dump state", dumps out the DATAFILE data, so we can see if it
    was all read in.)

-seed UNSIGNED_LONG (RNG seed). If you don't use this, running the app
   several times in rapid succession may generate the same PC info!

-dump (human-readable dump of generated PC).

-o OUTFILE. Save PC to given file, defaulting to stdout. If
   -dump is used and -o is used, the data is NOT sent to stdout,
   but instead goes only to the -o file.

-gender 0|1 where 0=female and 1=male
*/

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/proxy/pod/int.hpp>
#include <s11n.net/s11n/proxy/pod/double.hpp>
#include <s11n.net/s11n/proxy/pod/string.hpp>
#include <s11n.net/s11n/proxy/std/list.hpp>
#include <s11n.net/s11n/proxy/std/map.hpp>
#include <memory> // auto_ptr 

#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR

#include "argv_parser.hpp"

#include <stdlib.h> // random()
#include <ctime> // time()


typedef std::list<std::string> StringList;

/** Global list of available PC attributes. STR, DEX, etc. */
StringList attribute_list;

/** List of available kindred (races). */
StringList kindred_list;

/** map[attribute] = modifier */
typedef std::map<std::string,double> AttrModMap;

/** map[kindred][attribute] = modifier */
typedef std::map<std::string,AttrModMap> KindredModMap;

/**
   map[kindred][attribute] = modifier
*/
KindredModMap kindred_mods_map;

int rand_int(int min, int max)
{

	static unsigned long seed = 0L;
	if( 0L == seed )
	{
		seed = std::time(NULL);
		std::srand( seed );
	}
	if( max == min ) return min;
	if( max < min )
	{
		unsigned long tmp = max;
		max = min;
		min = tmp;
	}
	return min + (std::rand() % ((unsigned long) (max - min)+1) );
}


/**
   A player-character (PC) for the Enondas system.
*/
class enondas_pc
{
public:
        enondas_pc() : m_name("Unnamed"), m_kindred("human")
        {
                this->init();
        }

        explicit enondas_pc( const std::string & name, const std::string & race ) : m_name(name), m_kindred(race)
        {
                this->init();
        }

        ~enondas_pc() {}

        typedef std::map<std::string,int> attr_map;

        bool operator()( s11nlite::node_type & dest ) const
        {
                typedef s11n::node_traits<s11nlite::node_type> TR;
                TR::class_name( dest, "enondas_pc" );
                TR::set( dest, "gender", this->m_gender );
                TR::set( dest, "name", this->m_name );
                TR::set( dest, "kindred", this->m_kindred );
                s11n::map::serialize_map( dest, "attributes", this->m_attr );
                return true;
        }

        bool operator()( const s11nlite::node_type & src )
        {
                typedef s11n::node_traits<s11nlite::node_type> TR;
                this->m_name = TR::get( src, "name", this->m_name );
                this->m_kindred = TR::get( src, "kindred", this->m_kindred );
                s11n::map::deserialize_map( src, "attributes", this->m_attr );
                this->m_gender = TR::get( src, "gender", this->m_gender );
                return true;
        }

        attr_map & attr() { return this->m_attr; }
        const attr_map & attr() const { return this->m_attr; }

        std::string name() const { return this->m_name; }
        void name( const std::string & n ) { this->m_name = n; }

        std::string kindred() const { return this->m_kindred; }
        void kindred( const std::string & n ) { this->m_kindred = n; }

        int gender() const { return this->m_gender; }
        void gender( int g ) { this->m_gender = g; }


        int adds() const
        {

                int adds = 0;
                int at;
                attr_map bogo = this->m_attr; // to avoid having to use iterators
                // when doing the attr lookups (map[] is non-const).
		// Reminder: the (0!=at) check here to to handle the case that the
		// attribute names in the definition file aren't exactly those
		// which we hard-code here.
#define ADDS(A) at = bogo[A]; \
                if( 0 != at ) { \
			adds += (at < 9 ? ((9-at)*-1) : (at>12 ? (at-12) : 0)); \
		}
                ADDS("STR");
                ADDS("LK");
                ADDS("DEX");
                return adds;
#undef ADDS
        }

private:
        void init()
        {
                StringList::const_iterator cit = attribute_list.begin(),
                        cet = attribute_list.end();
                for( ; cet != cit; ++cit )
                {
                        this->m_attr[*cit] = 0;
                }
        }

        std::string m_name;
        std::string m_kindred;
        int m_gender; // 1 == male, 0 == female.
        attr_map m_attr;

};

#define S11N_TYPE enondas_pc
#define S11N_TYPE_NAME "enondas_pc"
#include <s11n.net/s11n/reg_s11n_traits.hpp>


/**
   Class to save/restore app-wide state.
*/
struct enondas_state
{

        // serialize
        template <typename NodeT>
        bool operator()( NodeT & dest ) const
        {
                typedef s11n::node_traits<NodeT> TR;
                TR::class_name( dest, "enondas_state" );
                s11n::list::serialize_list( dest, "attributes", attribute_list );
                s11n::list::serialize_list( dest, "kindred", kindred_list );
                s11n::map::serialize_map( dest, "kindred_modifiers", kindred_mods_map );
                return true;
        }

        // deserialize
        template <typename NodeT>
        bool operator()( const NodeT & src ) const
        {
                typedef s11n::node_traits<NodeT> TR;
                attribute_list.clear();
                s11n::list::deserialize_list( src, "attributes", attribute_list );
                s11n::list::deserialize_list( src, "kindred", kindred_list );
                s11n::map::deserialize_map( src, "kindred_modifiers", kindred_mods_map );
                return true;
        }
};

#define S11N_TYPE enondas_state
#define S11N_TYPE_NAME "enondas_state"
#include <s11n.net/s11n/reg_s11n_traits.hpp>

/**
   A human-friendly dump of a PC's state.
*/
void dump_pc( enondas_pc & pc, std::ostream & os )
{

        os << "Name:\t" << pc.name() << "\n";
        os << "Kindred: " << (pc.gender() ? "male" : "female") << " " << pc.kindred() << "\n";

        StringList::const_iterator cit = attribute_list.begin(),
                cet = attribute_list.end();
        std::string attr;
        for( ; cet != cit; ++cit )
        {
                attr = (*cit);
                os << attr << ":\t" << pc.attr()[attr] << "\n";
        }
        os << "Adds:\t" << pc.adds() << "\n";

//         s11nlite::save( pc, std::cout );

}


/**
   Rolls num d6. If drop_low is true then the lowest roll is discarded
   (used for 4d6/drop low attribute rolling system).
*/
size_t roll_d6( size_t num, bool drop_low = false )
{
        size_t accum = 0;
        size_t low = 6;
        size_t roll;
        for( size_t i = 0; i < num; i++ )
        {
                roll = rand_int( 1, 6 );
                low = (roll < low ? roll : low);
                accum += roll;
        }
        if( drop_low )
        {
                accum -= low;
                //CERR << "Dropping lowest die: " << low <<". Roll="<<accum<<"\n";
        }
        return accum;
}

/**
   Rolls attributes for the given pc, using roll_d6(dice,drop_low).
*/
void roll_attributes( enondas_pc & pc, size_t dice, bool drop_low )
{

        StringList::const_iterator cit = attribute_list.begin(),
                cet = attribute_list.end();
        double val;
        std::string attr;
        double mod;
        for( ; cet != cit; ++cit )
        {
                attr = *cit;
                val = roll_d6( dice, drop_low );
                mod = kindred_mods_map[pc.kindred()][attr];
                //CERR << "Attribute: " << attr << " = "<<val<<" (mod="<<mod<<")\n";
                val = val * mod;
                pc.attr()[attr] = static_cast<int>( val );
        }
        //pc.attr()["HP"] = pc.attr()["CON"];
}

static char const * default_output_format = "funtxt";

void show_help( char const * appName )
{
	std::cout << "enondas character generator\n"
		  << "libs11n sample application for generating random PCs for the Enondas roleplaying game.\n"
		  << "Usage: " << appName << " [options]\n"
		  << "OPTIONS:\n"
		  << "-? or --help\n\tShow this help and exit.\n"
		  << "-d FILE\n\tSets the input PC definition file [default=./enondas.s11n]\n"
		  << "-s SerializerClass\n\tSets the s11n serializer class for output [default="<<default_output_format<<"]\n"
		  << "-dice COUNT\n\tSets the number of dice to roll for each attribute [default=3]\n"
		  << "-droplow\n\tDrops the lowest roll from every stat roll (e.g., for use with --dice=4)\n"
		  << "-dump\n\tDump output to stdout in non-s11n format. -o is ignored. More human-readable\n"
		  << "-o OUTFILE\n\tFilename to save to [default=stdout]\n"
		  << "-name \"PC's NAME\"\n\tThe name of the generated PC [default=unnamed].\n"
		  << "-gender 0|1\n\tSpecifies the PC's gender, where 0=female and 1=male\n"
		  << "-kindred pc_race\n\tSets the PC kindred to pc_race, which MUST be defined in the -d file [default=human].\n"
		  << "-ds\n\t'dump state', dumps out the DATAFILE data, so we can see if it was all read in.\n"
		  << "-seed UNSIGNED_LONG\n\tRNG seed. If you don't use this, running the app several times in rapid succession may generate the same PC info!"

		  << std::endl;
}

#define NODEGETS(N,K,D) NTR::get( N, std::string(K), std::string(D) )
int main( int argc, char ** argv )
{
	cliutil::argv_map argmap;
	cliutil::unnamed_args_list unused;
	s11nlite::node_type argn;
	typedef s11nlite::node_traits NTR;
	cliutil::parse_args( argc, argv, 1, argmap, unused );

	cliutil::argv_map::const_iterator amit = argmap.begin();
	for( ; argmap.end() != amit; ++amit )
	{
		NTR::set( argn, (*amit).first, (*amit).second );
	}

	if( NTR::is_set( argn, "?" ) || NTR::is_set( argn, "help" ) )
	{
		show_help(argv[0]);
		return 0;
	}

        s11nlite::serializer_class( NODEGETS( argn, "s", default_output_format ) );


        std::string datafile = NODEGETS( argn, "d", "enondas.s11n" );

	typedef std::auto_ptr<enondas_state> SAP;
	SAP state( s11nlite::load_serializable<enondas_state>( datafile ) );
        if( ! state.get() )
        {
                CERR << "Error loading game data from '"<<datafile<<"'!\n";
                return 1;
        }

        if( NTR::is_set( argn, "ds" ) ) // dump state
        {
                CERR << "App state:\n";
                s11nlite::save( *state, std::cout );
        }


        rand_int(1,2); // kludge. that func seeds RNG on first run,
        // but i wanna force a specific seed below...
        unsigned long seed = NTR::get<ulong>( argn, "seed", ::time(NULL) );
        ::srandom( seed );

        enondas_pc pc;
        pc.name( NODEGETS( argn, "name", "unnamed" ) );
        pc.kindred( NODEGETS( argn, "kindred", "human" ) );
        pc.gender( NTR::get( argn, "gender", rand_int(0,1) ) );
        roll_attributes( pc, NTR::get( argn, "dice", 3 ), NTR::get( argn, "droplow", false ) );

//         COUT << "Character:\n";

        std::string ofile = NODEGETS( argn, "o", "" );
        if( ! ofile.empty() )
        {
                bool b = s11nlite::save( pc, ofile );
                CERR << (b ? "Saved" : "Error saving")
                     << " PC to file [" << ofile << "].\n";
        }
        else if( NTR::is_set( argn, "dump" ) )
        {
                dump_pc( pc, std::cout );
        }
        else
        {
                s11nlite::save( pc, std::cout );
        }

        //CERR << "Adds = " << pc.adds() << "\n";
        
        return 0;

}
