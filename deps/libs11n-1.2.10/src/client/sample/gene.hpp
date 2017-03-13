#ifndef gene_HPP_INCLUDED
#define gene_HPP_INCLUDED 1


#include <iostream>
#include <map>
#include <string>
#include <stdexcept>

/**
   The gene namespace contains my very first attempt at modelling
   information using genetics-like structures. It is not intended to
   provide a "complete" genetic algo framework, but to provide a basis
   off of which i can model some game info for use with genetic
   algorithms.

   License: public domain

   Author: stephan@s11n.net, August 2005
*/
namespace gene {


	/**
	   A rudimentary "gene" type. Stores a single value with
	   a client-side interpretation.

	   Most algorithms will require that ValueT be numeric: int,
	   long, and double are recommended, short and char are not
	   (some algos perform math which may easily over/underflow on
	   small numeric types).
	*/
	template <typename ValueT>
	class Gene
	{
	public:
		typedef ValueT value_type;

		/** Creates a gene with a default-constructed value.
		    Normally this would be 0.
		 */
		Gene() : m_val()
		{
		}

		~Gene()
		{
		}

		/** Copies rhs. */
		Gene( const Gene & rhs )
		{
			this->copy( rhs );
		}

		/** Copies rhs. */
		Gene & operator=( const Gene & rhs )
		{
			this->copy( rhs );
			return *this;
		}

		/**
		   Creates a gene with the given value.
		   
		   Note that this ctor is not explicit, allowing
		   implicit conversions from value_type to Gene.
		 */
		Gene( value_type val ) : m_val(val)
		{
		}

		/** Returns this->value(). */
		inline operator value_type() const
		{
			return this->value();
		}

		/** Returns this object's value. */
		inline value_type value() const
		{					
			return this->m_val;
		}

		/** Sets this object's value. */
		inline void value(value_type v)
		{
			this->m_val = v;
		}

		/** Returns (this->value() < rhs.value()). */
		inline bool operator<( const Gene & rhs )
		{
			return this->value() < rhs.value();
		}

		/** Returns (this->value() == rhs.value()). */
		inline bool operator==( const Gene & rhs )
		{
			return this->value() == rhs.value();
		}

		/** Returns !(this->operator==(rhs). */
		inline bool operator!=( const Gene & rhs )
		{
			return !(this->operator==(rhs) );
		}

	private:
		/** Copies rhs if rhs is not the same object as
		    this. */
		inline void copy(const Gene & rhs)
		{
			if( this != &rhs )
			{
				this->value( rhs.value() );
			}
		}
		value_type m_val;
	};

	/** Returns (os << g.value()). */
	template <typename T>
	inline std::ostream & operator<<( std::ostream & os, const Gene<T> & g )
	{
		return os << g.value();
	}

	/**
	   Sets g.value() from istr using T's istream operator. If
	   (!istr.good()) then g is not modified by this function.
	*/
	template <typename T>
	inline std::istream & operator>>( std::istream & istr, Gene<T> & g )
	{
		T v;
		if( (istr >> v) )
		{
			g.value(v);
		}
		return istr;
	}


	/**
	   Returns a random number in [min,max]. That is, between min
	   and max, inclusive. The underlying RNG implementation is
	   unspecified.

	   This function is the only one in the core gene library
	   which is not defined inline (because it's not a template
	   and the implementation is big/ugly enough to not want to
	   inline it). Because of this, clients must either link in
	   the objects/libs which implement this function or provide
	   thier own implementation which conforms to the behaviour
	   defined above.

	   Notes regarding the default implementation:

	   - If the difference between min and max is greater than C's
	   RAND_MAX, behaviour is undefined.

	   - If (min>max), then the function behaves as if it is called
	   with the arguments swapped: (max,min).

	   - The first time this function is called,
	   std::srand(std::time(NULL)) is called to seed the RNG.
	*/
	long rand( long min, long max );

	/**
	   A functor which uses gene::rand() to return random numbers
	   in a range.
	*/
	struct rng
	{
		long min;
		long max;
		rng( long _min, long _max ) : min(_min), max(_max) {}
		/** Sets min=0, max=1. */
		rng() : min(0), max(1) {}

		/** Returns rand(this->min,this->max). */
		long operator()() const
		{
			return ::gene::rand( this->min, this->max );
		}

		/** See rand(long,long). */
		long operator()( long _min, long _max ) const
		{
			return ::gene::rand( _min, _max );
		}
	};

	/**
	   Line the rng type, but generates number in a range of
	   doubles.

	   Don't depend on this class for generating high-resoultion
	   doubles!  This class is intended for use with "imprecise"
	   doubles, as in numbers ranging in the +/- millions, maybe
	   billions, area. At higher resolutions is might behave
	   "unexpectedly."
	*/
	struct rng_double
	{
	private:
		static const unsigned long scale = 100000000; // a conversion factor for forwarding to gene::rand(long,long)
	public:
		double min;
		double max;
		rng_double( double _min, double _max ) : min(_min), max(_max) {}
		/** Sets min=0.0, max=1.0. */
		rng_double() : min(0.0), max(1.0) {}


		/**
		   Returns a random number in [_min,_max].

		   ACHTUNG:

		   Due to the nature of floating point numbers, and
		   this implementation's abuse (reuse!) of a
		   non-floating-point based RNG, there will be
		   "rounding errors" or outright data loss at very
		   high precisions. On my box, only ranges with a
		   spread of no smaller than 0.0001 are of any use,
		   and ranges that small certainly won't be spread
		   very well. Smaller numbers simply get "lost in
		   translation". In any case, the higher the
		   precision, the more loss/inaccuracy there will be
		   in the range.
		 */
		double operator()( double _min, double _max ) const
		{
			return (1.0 * ::gene::rand( static_cast<long>(_min * scale ),
						    static_cast<long>(_max * scale ) )
				)
				/
				(1.0*scale)
				;
		}

		/**
		   Returns this->operator()(this->min,this->max).
		*/
		double operator()() const
		{
			return this->operator()( this->min, this->max );
		}

	};


	/**
	   Creates a gene by randomly selecting one of lhs or rhs.
	*/
	struct  breed_random_choice_f
	{
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs, const Gene<VT> & rhs ) const
		{
			return (1 == (::gene::rand(1,200) % 2)) ? lhs : rhs;
		}
	};

	/**
	   Creates a gene with the average values of lhs and rhs.

	   The value_type of the genes must support division by
	   2. e.g., it must be numeric.
	*/
	struct  breed_average_f
	{
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs, const Gene<VT> & rhs ) const
		{
			return Gene<VT>((lhs.value() + rhs.value()) / 2);
		}
	};

	/**
	   Creates a gene which takes on the weakest (lowest) value of
	   lhs or rhs. Note that genes which should reproduce rapidly
	   will want the breed_strongly_f functor instead of this one ;).

	   The value_type the genes must be numeric.
	*/
	struct  breed_weakly_f
	{
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs, const Gene<VT> & rhs ) const
		{
			return Gene<VT>( std::min( lhs.value(),rhs.value() ) );
		}
	};

	/**
	   Creates a gene which takes on the strongest (highest) value of lhs or
	   rhs.

	   The value_type of the genes must be numeric.
	*/
	struct  breed_strongly_f
	{
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs, const Gene<VT> & rhs ) const
		{
			return Gene<VT>( std::max( lhs.value(),rhs.value() ) );
		}
	};

	/**
	   A functor to breed genes using weighted averages.

	   The value_type of the genes must be numeric.
	*/
	struct  breed_weighted_average_f
	{
		/** See description in two-arg operator(). */
		enum Directions {
		Left = -2,
		Shrink = -1,
		Grow = 1,
		Right = 2
		};

		/** Weight of the "weak" gene. */
		int weak_weight;
		/** Weight of the "strong" gene. */
		int strong_weight;
		/** Breeding direction. See the two-arg operator() for details. */
		Directions direction;

		/**
		   Creates a breeder. The dir argument can be used to
		   modify the "direction" of the breeding: see the
		   two-arg operator() for how these values are
		   applied.

		   The lmod/hmod values, corresponding respectively to
		   this->weak_weight and strong_weight, are used to
		   determine how much more the left (or weakest) gene
		   should be weighted over the right (or strongest)
		   gene. For proper semantics, both lmod and hmod
		   should be positive, but this class does not enforce
		   that guideline.
		*/
		breed_weighted_average_f( int lmod = 1, int hmod = 2, Directions dir = Grow )
			: weak_weight(lmod),  strong_weight(hmod), direction(dir)
		{
		}

		/**
		   Creates a gene by using a weighted average of the values of
		   lhs and rhs. The average is calculated like so:
	   
		   ( (lowmultiplier * (min(lhs,rhs))) + (highmultiplier * (max(lhs,rhs))) )
		   / (lowmultiplier + highmultiplier)
	   
		   If 0 == (lowmultiplier + highmultiplier) then Gene(0.0) is
		   returned, rather than causing a divide by zero error.

		   This functor does not use any of this->{weak_weight,direction,strong_weight}.
		*/
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs,
					  const Gene<VT> & rhs,
					  typename Gene<VT>::value_type lowmultiplier,
					  typename Gene<VT>::value_type highmultiplier ) const
		{
			if( 0 == (lowmultiplier + highmultiplier) )
			{
				return Gene<VT>();
			}
			if( lhs == rhs ) return lhs;
			const VT vl = lhs.value();
			const VT vr = rhs.value();
			return Gene<VT>(
					     (
					      (lowmultiplier * std::min( vl, vr ))
					      + (highmultiplier * std::max( vl, vr ))
					      )
					     / (lowmultiplier + highmultiplier)
					     );
		}

		/**
		   Creates a gene by using a weighted average of the
		   values of lhs and rhs. Note the different argument
		   order (and semantics!) from the other four-arg operator()!

		   The average is calculated like so:
		   
		   ( (leftmultiplier * lhs) + (rightmultiplier * rhs) )
		   / (leftmultiplier + rightmultiplier)
		   
		   If 0 == (leftmultiplier + rightmultiplier) then Gene(Gene(VT())) is
		   returned, rather than causing a divide by zero error.

		   This functor does not use any of this->{weak_weight,direction,strong_weight}.
		*/
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs,
					  typename Gene<VT>::value_type leftmultiplier,
					  const Gene<VT> & rhs,
					  typename Gene<VT>::value_type rightmultiplier ) const
		{
			if( 0 == (leftmultiplier + rightmultiplier) )
			{
				return Gene<VT>(VT());
			}
			if( lhs == rhs ) return lhs;
			return Gene<VT>(
					     (
					      (leftmultiplier * lhs.value())
					      + (rightmultiplier * rhs.value())
					      )
					     / (leftmultiplier + rightmultiplier)
					     );
		}

		/**
		   Uses this->{weak_weight,strong_weight,direction} to breed a
		   weighted average of lhs and rhs.  

		   This function's behaviour is as follows:

		   "Weighed more strongly" means that the "stronger"
		   gene (as defined by direction: see below) is
		   multiplied by this->strong_weight, and the "weaker" gene
		   is multiplied by this->weak_weight. The average is then
		   calculate by adding those and dividing by
		   (weak_weight+strong_weight).

		   "Direction" of breeding:

		   - If (this->direction==Left), lhs is weighted more
		   strongly than rhs.

		   - If (this->direction==Right), rhs is weighted more.

		   - If (this->direction==Grow), the stronger of the
		   (lhs,rhs) is weighed more strongly than the weaker.

		   - If (this->direction==Shrink), the weaker of the
		   (lhs,rhs) is weighed more strongly than the
		   stronger. This will result in an overall stronger
		   gene than the original weaker one, but one which is
		   nearer the original weak gene than the original
		   strong gene. Contrast this with Grow, which will
		   produce a gene which is weaker than the original
		   strong gene, but lies closer to the original
		   stronger gene than the original weak gene.

		   This function throws a std::range_error if
		   this->direction is not a value from the Directions
		   enum.
		*/
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs, const Gene<VT> & rhs ) const
		{
			switch( this->direction )
			{
			  case Left:
				  return this->operator()( lhs, this->strong_weight, rhs, this->weak_weight );
			  case Shrink:
				  return this->operator()( lhs, rhs, this->strong_weight, this->weak_weight );
			  case Grow:
				  return this->operator()( lhs, rhs, this->weak_weight, this->strong_weight );
			  case Right:
				  return this->operator()( lhs, this->weak_weight, rhs, this->strong_weight );
			  default:
				  throw std::range_error( "breed_weighted_average_f::operator(): breeding direction out of bounds: must be one of: Left, Shrink, Grow, Right." );
			}
		}
	};



	/**
	   Creates a gene which takes makes a weighted average of lhs and rhs by
	   counting the higher of the two twice, adding the lower, and
	   dividing by 3.

	   The value_type of the genes must be numeric.
	*/
	struct  breed_strong_average_f
	{
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs, const Gene<VT> & rhs ) const
		{
			return breed_weighted_average_f()( lhs, rhs, 1, 2 );
		}
	};

	/**
	   Creates a gene which takes makes a weighted average of lhs
	   and rhs by counting the lower of the two twice, adding the
	   higher, and dividing by 3.

	   The value_type of the genes must be numeric.
	*/
	struct  breed_weak_average_f
	{
		template <typename VT>
		Gene<VT> operator()( const Gene<VT> & lhs, const Gene<VT> & rhs ) const
		{
			return breed_weighted_average_f()( lhs, rhs, 2, 1 );
		}
	};




	/**
	   Populates a target iterator with the results of breeding
	   two chains of input iterators.

	   IterT must be a forward iterator. [begin1,end1) must be the same
	   size as [begin2,end2), and this function stops processing at the
	   first end iterator it meets, whether it belongs to the first or
	   second iterator pair. FuncT must be defined like this:

	   Gene FUNC( const Gene &, const Gene & );

	   It may be a functor or a function pointer.

	   InsertInterT must point to a range big
	   enough to hold all items, or must be an insert iterator. 

	   Returns the number of items bred/inserted.
	*/
	template <typename IterT, typename FuncT, typename InsertIterT>
	size_t breed_lists( IterT begin1, IterT end1,
			    IterT begin2, IterT end2,
			    FuncT breedfunc,
			    InsertIterT insertat )
	{
		size_t ret = 0;
		for( ;
		     (end2 != begin2) && (end1 != begin1);
		     ++begin1, ++begin2, ++ret )
		{
			*insertat = breedfunc( *begin1, *begin2 );
			++insertat;
		}
		return ret;
	}


	/**
	   Genome is a wrapper for holding maps of named genes.
	*/
	template <typename GeneType, typename KeyType = std::string>
	class Genome
	{
	public:
		typedef KeyType key_type;
		typedef GeneType gene_type;
		typedef typename gene_type::value_type value_type;
		typedef std::map<key_type,gene_type> gene_map;

		typedef typename std::map<key_type,gene_type>::iterator iterator;
		typedef typename std::map<key_type,gene_type>::const_iterator const_iterator;

		/** Creates an empty Genome. */
		Genome()
		{
		}

		/**
		   Returns the named gene. The behaviour when the gene
		   does not exist depends on the gene_map type. When
		   that is a std::map then the gene will be
		   default-constructed.
		*/
		gene_type & operator[]( const key_type & key )
		{
			return this->genes()[key];
		}

		/**
		   Returns a copy of the named gene, throwing a
		   std::range_error if it does not exist.
		*/
		gene_type operator[]( const key_type & key ) const
		{
			const_iterator it = this->genes().find( key );
			if( this->genes().end() == it )
			{
				throw std::range_error( "Genome::operator[]("+key+"): gene not found" );
			}
			return (*it).second;
		}

		/**
		   Copies genes() from rhs.
		*/
		explicit Genome( const gene_map & rhs ) : m_g(rhs)
		{
		}

		~Genome()
		{
		}

		Genome & operator=(const Genome & rhs)
		{
			this->copy(rhs);
			return *this;
		}

		Genome(const Genome & rhs)
		{
			this->copy(rhs);
		}

		gene_map & genes()
		{
			return this->m_g;
		}
	
		const gene_map & genes() const
		{
			return this->m_g;
		}

		/** Returns an iterator to the first gene. */
		iterator begin() { return this->genes().begin(); }
		/** Returns a const iterator to the first gene. */
		const_iterator begin() const { return this->genes().begin(); }

		/** Returns an iterator to the after-the-end gene. */
		iterator end() { return this->genes().end(); }

		/** Returns a const iterator to the after-the-end gene. */
 		const_iterator end() const { return this->genes().end(); }

		/** Swaps genes() with rhs. */
		void swap( gene_map & rhs )
		{
			this->genes().swap( rhs );
		}

		/** Swaps genes() with rhs. */
		void swap( Genome & rhs )
		{
			this->swap( rhs.genes() );
		}

		/**
		   Removes all genes.
		*/
		void clear()
		{
			this->m_g.clear();
		}

		/**
		   Returns true if genes() contains the named gene,
		   else false.
		*/
		bool has_gene( const key_type & key ) const
		{
			return this->genes().end() != this->genes().find(key);
		}

	private:
		/** Copies genes from rhs if rhs is not this object. */
		void copy(const Genome & rhs)
		{
			if( this != &rhs )
			{
				this->genes() = rhs.genes();
			}
		}
		gene_map m_g;
	};


	/**
	   A functor to mutate genes randomly using a given range of
	   randomness. It provides several convenience overloads so
	   that it can be used on individual genes or genomes.

	   The main intention of this class is to mutate genes by
	   relatively small amounts: small fractions of a point.
	*/
	template <typename GeneType, typename KeyType = std::string>
	struct range_mutator_f
	{
		typedef GeneType Gene;
		typedef Genome<Gene,KeyType> GenomeType;

		typedef typename GenomeType::gene_map GenomeMap;
		typedef typename GenomeMap::value_type GenomePair;

		/**
		   Sets the low, high, and scale values.

		   If (s == 0) then any operators in this class might
		   cause a divide by zero error.

		   Example usage:

		   range_mutator_f< Gene<double> > r(-10,20,10000);

		   r( mygene ) will mutate mygene's value within a
		   range of (random[-10..+20]/10000).


		   A scale of 0 is illegal, and will likely cause a
		   floating-point exception eventually.
		*/
		range_mutator_f( int l, int h, unsigned int s )
			: low(l), high(h), scale(s)
		{}

		/**
		   A mutator with a range of +/-1%.
		*/
		range_mutator_f()
			: low(-1), high(1), scale(100)
		{}


		int low;
		int high;
		unsigned int scale;

		/**
		   Returns gene::rng(this->low,this->high).
		*/
		int rng() const
		{
			return ::gene::rand( this->low, this->high );
		}

		/**
		   Modifies g by a random double value in the
		   range (this->rng() / this->scale).

		   All other operator() overloads are implemented in
		   terms of this one.
		*/
		void operator()( Gene & g ) const
		{
			double d = g.value();
			d += ( (double)this->rng() / (double)this->scale );
			g.value( d );;
		}

		/**
		   Makes a copy of g and mutates it.
		*/
		Gene operator()( const Gene & g ) const
		{
			Gene ret(g);
			this->operator()( (Gene &)ret );
			return ret;
		}

		/**
		   Mutates p.second.
		*/
		void operator()( GenomePair & p ) const
		{
			Gene & g = p.second;
			this->operator()( (Gene &)g );
		}

		/**
		   Makes a copy of p.second and mutates it.
		*/
		Gene operator()( const GenomePair & p ) const
		{
			Gene g = p.second;
			this->operator()( (Gene &)g );
			return g;
		}

		/**
		   Mutates each gene in g.
		*/
		void operator()( GenomeMap & g ) const
		{
			typedef typename GenomeType::iterator GIT;
			GIT b = g.begin();
			for( ; g.end() != b; ++b )
			{
				this->operator()( (Gene &)((*b).second) );
			}
		}

		/**
		   Mutates each gene in g.genes().
		*/
		void operator()( GenomeType & g ) const
		{
			this->operator()( g.genes() );
		}

		/**
		   Makes a copy of g and mutates each gene in that copy.
		*/
		GenomeType operator()( const GenomeType & g ) const
		{
			GenomeType ret(g);
			this->operator()( (GenomeType &)ret );
			return ret;
		}
	};



	/**
	   A functor to wrap another functor to call
	   on the .first OR .second member of a pair.

	   This implementation works on the .first element and a
	   specialization provides selection of .second.
	*/
	template <typename FunctorT, bool First = true>
	struct pair_selector_fwd_f
	{
		/** Functor to be applied by operator().
		    Must support:

		    IgnoredType func( SomeT & ) const;

		    where SomeT is the type of some_pair.first, as
		    passed to operator().
		*/
		FunctorT func;

		explicit pair_selector_fwd_f( FunctorT f ) : func(f)
		{
		}

		/**
		   Returns this->func( p.first ).
		*/
		template <typename PairT>
		void operator()( PairT & p ) const
		{
			this->func( p.first );
		}

		/**
		   Returns this->func( p.first ).
		*/
		template <typename PairT>
		void operator()( const PairT & p ) const
		{
			this->func( p.first );
		}

	};

	/**
	   Specialized to pass on the .second member
	   of a pair to a FunctorT.
	*/
	template <typename FunctorT>
	struct pair_selector_fwd_f<FunctorT,false>
	{
		/**
		   Functor to be applied by operator().
		   Must support:

		   IgnoredType func( SomeT & ) const;

		   where SomeT is the type of some_pair.second,
		   as passed to operator().
		*/
		FunctorT func;
		explicit pair_selector_fwd_f( FunctorT f ) : func(f)
		{
		}

		/**
		   Returns this->func( p.second ).
		*/
		template <typename PairT>
		void operator()( PairT & p ) const
		{
			this->func( p.second );
		}


		/**
		   Returns this->func( p.second ).
		*/
		template <typename PairT>
		void operator()( const PairT & p ) const
		{
			this->func( p.second );
		}
	};


	/**
	   Convenience func to create a pair_selector_fwd_f which
	   calls f(somepair.first).

	   e.g.

	   std::for_each( map.begin(), map.end(), pair_second_func( MyFunctor() ) );

	   will apply MyFunctor using each .first member of the map. Remember
	   that map keys are const, so they can't be mutated! Thus MyFunctor
	   must be able to accept const references or copies of keys.
	*/
	template <typename FunctorT>
	pair_selector_fwd_f<FunctorT,true>
	pair_first_func( FunctorT f )
	{
		return pair_selector_fwd_f<FunctorT,true>(f);
	}

	/**
	   Convenience func to create a pair_selector_fwd_f which
	   calls f(somepair.second).

	   e.g.

	   std::for_each( map.begin(), map.end(), pair_second_func( MyFunctor() ) );

	   will apply MyFunctor to each member of the map.  MyFunctor
	   must accept either a (const SecondType &) or (SecondType &),
	   depending on the context's contestness.
	*/
	template <typename FunctorT>
	pair_selector_fwd_f<FunctorT,false>
	pair_second_func( FunctorT f )
	{
		return pair_selector_fwd_f<FunctorT,false>(f);
	}


// 	/**
// 	*/
// 	template <typename BreederFunc>
// 	struct breeder_f
// 	{
// 		typedef BreederFunc functor_t;
// 		functor_t func;
// 		breeder_f( functor_t f ) : func(f)
// 		{
// 		}

// 		template <typename VT>
// 		Gene<VT> operator()( const Gene<VT> & lhs, const Gene<VT> & rhs )
// 		{
// 			return this->func( lhs, rhs );
// 		}
// 	};


	/**
	   This function breeds all common genes in lhs.genes() and
	   rhs.genes(), returning a new Genome object. The lhs Genome
	   is used as the basis for the list of genes. Any lhs genes
	   which are not found in rhs are skipped. This condition can
	   be checked for by comparing the size of the returned
	   object's genes() to that of lhs.

	   BreedFuncT must have a signature compatible with:

	   GeneT funcname( const GeneT &, const GeneT & ) [const];

	   and should return a gene which is a "combination" of the
	   two arguments, though how to "combine" them is purely up to
	   the function - it may randomly select, merge, average,
	   etc.

	*/
	template <typename GeneT, typename KeyType, typename BreedFuncT>
	Genome< GeneT, KeyType >
	breed_genomes( const Genome<GeneT,KeyType> & lhs,
		       const Genome<GeneT,KeyType> & rhs,
		       BreedFuncT breedfunc )
	{
		typedef Genome<GeneT,KeyType> GenomeT;
		typedef typename GenomeT::const_iterator GIT;
		GenomeT ret;
		GIT end1 = lhs.end();
		GIT begin1 = lhs.begin();
		GIT end2 = rhs.end();
		GIT rhg; // right-hand gene
		for( ; end1 != begin1; ++begin1 )
		{
			rhg = rhs.genes().find( (*begin1).first );
			if( end2 == rhg ) { continue; }
			ret[(*begin1).first] = breedfunc( (*begin1).second, (*rhg).second );
		}
		return ret;
	}


} // namespace gene


#endif // gene_HPP_INCLUDED
