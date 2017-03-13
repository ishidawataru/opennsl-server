#include <s11n.net/shellish/plugin.hpp>
#include <s11n.net/shellish/shellish_config.hpp> // config macros

#include <iostream>
#ifndef CERR
#define CERR std::cerr << __FILE__ << ":" << std::dec << __LINE__ << " : "
#endif

namespace shellish { namespace plugin {

	::shellish::path_finder & path()
	{
		static ::shellish::path_finder bob;
		static bool donethat = false;
		if( (!donethat) && (donethat=true) )
		{
			bob.add_path( shellish_CONFIG_PLUGINS_PATH );
			bob.add_extension( shellish_CONFIG_DLL_EXTENSION );
		}
		return bob;
	}

	std::string find( const std::string & basename )
	{
		return path().find( basename );
	}

} } // namespace


#if !defined(WIN32) || !(WIN32)
#  if shellish_CONFIG_HAVE_LIBLTDL || shellish_CONFIG_HAVE_LIBDL
#    include "plugin.dl.cpp"
#  else
#    include "plugin.noop.cpp"
#  endif // libdl/ltdl?
#else
#    include "plugin.win32.cpp"
#endif // WIN32?
