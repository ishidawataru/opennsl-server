////////////////////////////////////////////////////////////////////////
// A "supermacro"-style classloader registration snippet for the
// s11n::fac layer.
////////////////////////////////////////////////////////////////////////
// Macros to be set before including this file:
//
// SHELLISH_FACREG_TYPE: type to be registered.
//
// SHELLISH_FACREG_TYPE_NAME: string name to register SHELLISH_FACREG_TYPE with.
//
// SHELLISH_FACREG_INTERFACE_TYPE: base-most type of SHELLISH_FACREG_TYPE.
// Defaults to SHELLISH_FACREG_TYPE.
//
// OPTIONAL/special-case macros:
//
// SHELLISH_FACREG_TYPE_IS_ABSTRACT:
// This macro should be used only when SHELLISH_FACREG_TYPE is an abstract type.
// If SHELLISH_FACREG_TYPE is an abstract base class, define this
// macro to any value. If this is set then a no-op specialization
// of P::Hook::create_hook<> is installed so no calls to 
// new SHELLISH_FACREG_TYPE will be made by the default factories. 
//
//
// All of the above macros are #undef'd by this file, so it may be
// included multiple times in succession.
////////////////////////////////////////////////////////////////////////

#ifndef SHELLISH_FACREG_TYPE
#  error "You must define SHELLISH_FACREG_TYPE before including this file."
#endif

#ifndef SHELLISH_FACREG_TYPE_NAME
#  error "You must define SHELLISH_FACREG_TYPE_NAME before including this file."
#endif

#ifndef SHELLISH_FACREG_INTERFACE_TYPE
#  define SHELLISH_FACREG_INTERFACE_TYPE SHELLISH_FACREG_TYPE
#endif


#  ifdef SHELLISH_FACREG_TYPE_IS_ABSTRACT
// Install a specialization of Factory's default factory type,
// so that we won't try to instantiate a SHELLISH_FACREG_TYPE object.
// For the non-abstract case we will rely on the default create_hook
// implementation (or a client-installed specialization).
namespace s11n { namespace fac {
	/**
	   create_hook specialization to install a no-op
	   factory. Intended for use with abastract types.
	*/
	template <>
	struct create_hook< SHELLISH_FACREG_INTERFACE_TYPE , SHELLISH_FACREG_TYPE >
	{
		typedef SHELLISH_FACREG_INTERFACE_TYPE * result_type;
		typedef SHELLISH_FACREG_TYPE actual_type;
		/** Returns 0. */
		static result_type create()
		{
			return 0;
		}
	};
} } // namespace s11n::fac
#  endif // SHELLISH_FACREG_TYPE_IS_ABSTRACT

namespace {

#  ifndef shellish_FACTORY_REG_CONTEXT_DEFINED
#  define shellish_FACTORY_REG_CONTEXT_DEFINED 1
	///////////////////////////////////////////////////////////////
	// we must not include this more than once per compilation unit
	/**
	   A unique (per Context/per compilation unit) space to assign
	   a bogus value for classloader registration purposes (see
	   the classloader docs for a full description of how this
	   works).
	*/
	template <typename Context>
	struct shellish_factory_reg_context
	{
		shellish_factory_reg_context()
		{
			if( true == placeholder ); // weird workaround, just to reference the var. 
		}
		/**
		   Placeholder variable for automatic factory
		   registration.
		*/
		static bool placeholder;
	};
#  endif // !shellish_FACTORY_REG_CONTEXT_DEFINED

	/**
	   Placeholder variable for automatic factory registration.
	*/
	template <>
	bool
	shellish_factory_reg_context< SHELLISH_FACREG_TYPE >::placeholder= (
 				 ::shellish::fac::register_subtype<
					 SHELLISH_FACREG_INTERFACE_TYPE,
					 SHELLISH_FACREG_TYPE >(
							    std::string(SHELLISH_FACREG_TYPE_NAME),
							    ::shellish::fac::create_hook<
								    SHELLISH_FACREG_INTERFACE_TYPE,
								    SHELLISH_FACREG_TYPE>::create
						    ),
				 true);
	// DAMN, that's fugly!
} // anon namespace

#ifdef SHELLISH_FACREG_TYPE_IS_ABSTRACT
#  undef SHELLISH_FACREG_TYPE_IS_ABSTRACT
#endif
#undef SHELLISH_FACREG_TYPE
#undef SHELLISH_FACREG_INTERFACE_TYPE
#undef SHELLISH_FACREG_TYPE_NAME
