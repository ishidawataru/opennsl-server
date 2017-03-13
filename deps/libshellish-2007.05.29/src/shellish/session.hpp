#ifndef shellish_s11n_SESSION_HPP_INCLUDED
#define shellish_s11n_SESSION_HPP_INCLUDED 1

#include <string>

namespace shellish {


        /**
           Saves aliases, command history (if available) and
           environment to the given file.  Returns true on success,
           false on error. It may propagate an exception in the face
           of an error.

	   If includeEnv is false then the env() is excluded from the
	   export. This is often desireable, to keep values loaded
	   from session files from shadowing important environment
	   vars, like LD_LIBRARY_PATH (which might be used for plugin
	   lookups).

	   If shellish is built without s11n support, this function
	   throws a std::exception (or subclass) explaining the lack
	   of the feature.
        */
        bool save_session( const std::string & filename, bool includeEnv = true );

        /**
           Loads aliases, command history (if available) and
           environment from the given file.  Returns true on success,
           false on error. It may propagate an exception in the face
           of an error.

	   If shellish is built without s11n support, this function
	   throws a std::exception (or subclass) explaining the lack
	   of the feature.
         */
        bool load_session( const std::string & filename );



} // namespace


#endif // shellish_s11n_SESSION_HPP_INCLUDED
