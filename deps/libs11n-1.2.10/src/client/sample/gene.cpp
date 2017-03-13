////////////////////////////////////////////////////////////////////////
// Implementation for non-inlined/template ::gene code.
// Demonstration of s11n proxies for gene's template types.
//
// License: public domain
// Author: stephan@s11n.net
////////////////////////////////////////////////////////////////////////

#include <iostream> // cout/cerr
#include <cstdlib> // rand()
#include <ctime> // time()

#include "gene.hpp"

namespace gene {

	long rand( long min, long max )
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
		return ( min + ( std::rand() % ((unsigned long) (max - min)+1) ) );
	}

} // namespace

////////////////////////////////////////////////////////////////////////
// End of implementation code: the rest is all demonstration...
////////////////////////////////////////////////////////////////////////

#include <s11n.net/s11n/s11nlite.hpp>
#include <s11n.net/s11n/micro_api.hpp>
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR

#include "gene_s11n.hpp" // s11n proxies for gene:: types.

#if gene_USE_HEAVYWEIGHT_S11N
// Promote our Gene-related PODs to full-fledged Serializables:
#  include <s11n.net/s11n/proxy/pod/string.hpp> // proxy for gene keys
#  include <s11n.net/s11n/proxy/pod/double.hpp> // proxy for gene data
#  include <s11n.net/s11n/proxy/pod/int.hpp> // proxy for genome keys and gene data
#endif

typedef gene::Gene<int> Gene;
typedef gene::Genome<Gene> Genome;
int main( int argc, char ** argv )
{
	s11nlite::serializer_class( "funtxt" ); // XML won't work with gene::Genome<GeneType,NUMERIC_TYPE> due to numeric keys

	Genome him;
	him["geekness"] = 5;
	him["taste_in_clothes"] = -2;

	Genome her;
	her["geekness"] = 0;
	her["taste_in_clothes"] = 7;

	Genome them = gene::breed_genomes( him, her, gene::breed_average_f() );

	typedef s11nlite::micro_api<Genome> MG;
	MG mg("parens");
	

	CERR << "Him == \n";
	mg.save( him, std::cout );

	CERR << "Her == \n";
	mg.save( her, std::cout );

	CERR << "Them == \n";
	// Use a buffer to "save" them, to show that we can actually
	// deserialize the Genome...
	mg.buffer( them );
	s11n::cleanup_ptr<Genome> load( mg.load_buffer() );
	if( ! load.get() )
	{
		CERR << "Error deserializing Genome!\n";
		return 2;
	}
	mg.save( *load, std::cout );


	CERR << "Now for int-indexed, double-valued genes...\n";
	typedef gene::Gene<double> MyGene;
	typedef gene::Genome<MyGene,int> MyNome;
	MyNome n2;
	MyNome n3;
	const int max = 10;
	for( int i = 1; i <= max; i += 2 )
	{
		if( i % 3 == 0 )
		{
			n2[i] = (7654321.012345*i)+(0.1*i);
			n3[i] = 20.83 * i;
		}
		else
		{
			n2[i] = 1.0 *i + (0.1*i);
			n3[i] = 2.0 *i;
		}
		n2[i+1] = -n2[i];
		n3[i+1] = n2[i];
	}
	s11nlite::micro_api<MyNome> micd("parens");
	CERR << "Two parent genomes:\n";
	micd.save( n2, std::cout );
	micd.save( n3, std::cout );
	MyNome child( gene::breed_genomes( n2, n3, gene::breed_random_choice_f() ) );
	micd.buffer( child );
	s11n::cleanup_ptr<MyNome> cld( micd.load_buffer() );
	if( ! cld.get() )
	{
		CERR << "error loading MyNome :(\n";
		return 3;
	}
	CERR << "A child of the above parents:\n";
	micd.save( *cld, std::cout );

	return 0;
}
