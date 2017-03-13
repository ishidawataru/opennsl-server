// environment.h
#ifndef shellish_ENVIRONMENT_H_INCLUDED
#define shellish_ENVIRONMENT_H_INCLUDED 1

// License: Public Domain
// Author: stephan@s11n.net

#include <s11n.net/shellish/property_store.hpp>

namespace shellish {

/**
environment is an OO front-end to getenv()-like functionality. It provides some
type-casting-like functionality, so you can easily read in environment vars which have,
e.g., numeric values.
*/
class environment : public property_store
{
 public:
        /**
           Returns a reference to a shared object containing the
           keys/values from the given char ** (defaults to 'environ',
           from &lt;unistd.h&gt;, which represents the shell's
           environment). The input array is not changed, but is not
           const because environ is not const. See 'man 5 environ' for
           info about the default environment array.

           Calling it with a non-0 parameter will clear the current
           environment settings and parse them from the passed-in
           array, which is expected to be in the form:

<pre>
"KEY1=VAL1",
"KEY2=VALUE TWO",
...
0
</pre>

             The returned object has propagate_sets() == true.

             Tip: s11n::s11n_node::serialize_properties() can
             de/serialize this object.
         */
        static environment & env( char ** = 0 );


        /**
           Re-implemented to pass the key/val to ::setenv() (so
           subshells can access them).

           Changes are not written back to the global shell
           environment (using setenv()) unless propagate_sets() has
           been set to true.
         */
        virtual void set_string( const std::string &key, std::string val );

        /**
           Overridden to call unsetenv() if propagate_sets() has been set.

           Note that the parent class' clear_properties() does not
           call unset(), so calling clear_properties() on this object
           will not hose the global environment even if
           propagate_sets() is true.
        */
        virtual bool unset( const std::string & key );

        /**
           Parsed env vars out of text, replacing them with their
           values. Accepts variable tokens in these forms:

           ${VAR}
           $VAR

           e.g., $foo corresponds to the value returned by
           this->get_string("foo").

           Referencing a variable which is not set does not
           expand the variable to an empty value: it is left
           as-is. Thus expanding $FOO when "FOO" is not set
           will result in "$FOO".

           To get a dollar sign into the resulting string, escape
           it with a single backslash: this keeps it from being
           parsed as a $variable.
         */
        std::string expand_vars( const std::string & text ) const;

        /**
           Exactly like expand_vars() but directly modifies
           the input string. Returns the number of variables
           expanded.
        */
        size_t expand_vars_inline( std::string & text ) const;

        /**
           If set to true then calls to set_string() will
           also make a call to setenv(), effectively propagating
           changes back into the main environment.

           The default is false.
        */
        void propagate_sets( bool );

       /**
           Does conventions-compliant tilde/HOME expansion on the
           given input.

           Returns true if it modifies the string, else false.

           Bug: it only works on single tokens. That is,
           the input must be only one token long, where whitespace
           delimits tokens.
         */
          bool expand_tilde_home( std::string & ) const;

        environment();
        virtual ~environment();

 private:
        environment( const environment & ); // intentionally unimplemented.
        bool m_propagate;
};

} // namespace shellish
#endif // shellish_ENVIRONMENT_H_INCLUDED
