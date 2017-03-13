#ifndef shellish_ARGV_PARSER_H
#define shellish_ARGV_PARSER_H 1

// Author: stephan@s11n.net
// License: Public Domain

#include <s11n.net/shellish/property_store.hpp>
namespace shellish {
        /**
           argv_parser is an object for parsing command line options.

           Use it like this:

           <pre>
#include &lt;iostream&gt;
#include "argv_parser.h"

#define VERBOSE if( opts.get( "verbose", false ) ) cerr

int main( int argc, char **argv )
{
	argv_parser & opts = argv_parser::args( argc, argv );
        opts.set( "dosomething", ! opts.get_bool( "donothing", false ) );
        if( ! opts.get( "dosomething", true ) ) { exit( 0 )); }
        int myint = opts.get( "width", 20 );
        double myangle = opts.get( "angle", 0.0 );

        VERBOSE << "This is a verbose message." << endl;
        return opts.get( "errorcode", 0 );
}
</pre>
           (that code's just off the top of my head - it may not compile as-is.)

           Note that client code outside of main() can then get access
           to the args via the static function args():

           <pre>
argv_parser & opts = argv_parser::args();
           </pre>

           This can be used, e.g., by DLL init code to read the arguments passed
           to main().

           See the property_store object's API for a full list of accessor functions.
           
           Supported command-line formats:
           <pre>
-foo bar [that is, -foo == "bar"]
is the same as:
[--foo bar] [-foo=bar] [--foo=bar]

-foo -bar false [-foo == true, -bar == false]
-foo -bar=false [same]
--double-dashed "some string value"
--double-dashed="some string value" [same as previous line]
           </pre>

           Whether you use single or double dashes is irrelevant, and calls to
           get() may leave off the leading dashes. e.g., in this call:

           opts.get_string( "foo" ); // WITHOUT a dash

           the following options are searched:
<ol>
<li>foo
<li>-foo
<li>--foo
</ol>

           so the above call may actually result in getting the value from -foo or
           --foo. This is a potential logic problem if your application uses two
           semantically-different, like-named arguments like -force and
           --force. In this case a call to get( "force" ) would find -force
           and not --force. Use get( "-force" ) or get("--force") to
           avoid this ambiguity. The dashed versions of an argument are only
           sought if get() is called without a dash before the key. Thus calling
           get("-force") would never match "--force".

           These dash-searching rules also apply to is_set().

           A note to Qt users: call args() on this object before calling
           QApplication a(argc, argv ), or QApplication will steal any argument
           called -name (and possibly others), removing it from argv. i.e.,
           if you do not call args() on this object first, QApplication may
           steal arguments so you'll never see them. Been there, done that.

           Known Bugs and/or gotchyas:

           Negative numbers:

           <pre>
--boolflag -123=something
           </pre>

           will be parsed as:

           <pre>
[--boolflag == true] [-123=something]
           </pre>

           Search the .cpp file for 'negative' and you'll find where this bug lives.
           Since numeric arguments are so rare this is not such a big deal, i think.
           i can think of only a few real-world examples which use args like -1:
           ssh, [GNU] xargs, head, tail, lpr, ... okay, maybe they aren't so uncommon :/

           Along those same lines:

           <pre>
--bool -234 --foobar
           </pre>
           will be parsed as:
           <pre>
[--bool == -234] [--foobar == true]
           </pre>

           Which i consider to be correct for most cases. If you want to set
           --bool to a negative number use: --bool=-123
           If you want to force a boolean value in this case:

           <pre>
--bool=true -234 546
           </pre>

           parses as:

           <pre>
--bool=true
-234=456
           </pre>

           i hate the inconsistency this adds, though. :/
        */

        class argv_parser : public property_store
        {
        public:
                /**
                   Creates a new parser using the given
                   arguments array and arg count.
                */
                argv_parser( int argc, char *argv[], int startAt=0 );
                argv_parser();
                virtual ~argv_parser();

                /**
                   args( int, char * ) should be called once from your
                   main() if you want clients to be able to use args()
                   to get at them.
                */
                static argv_parser & args( int argc, char *argv[] );

                /**
                   Returns the object containing the arguments
                   supplied to args(int,char**).
                */
                static argv_parser & args();

                /**
                   get/set_help() text for a given key.
                */
                virtual void set_help( const std::string &key, const std::string &text );

                /**
                   Returns the help text associated with key.
                 */
                virtual const std::string get_help( const std::string &key ) const;

                /**
                   Re-implemented to check keys -FOO and --FOO if key FOO is
                   not found.
                */
                virtual bool is_set( const std::string &key ) const;

                /**
                   get_string() is overridden to add a special case to
                   all get() calls made via the property_store API: if
                   a get() function is called with a key which does
                   not start with a dash (-) character and they key
                   cannot be found in our list then -key and --key
                   will be tried.

                   This means that, assuming the above sample code is
                   in place, the following would work:

<pre>
~/ > myapp --foo=17.34
...
double d = opts.getDouble( "foo", 0.0 ); // d == 17.34
</pre>

                   As will this:
<pre>
opts.set( "--mykey", "myvalue" );
...
cerr << "mykey="<< opts.get_string( "mykey" ) << endl;
</pre>

                   Note, however, that command-line arguments passed
                   without a leading dash are not treated as
                   arguments, and will not be inside this object if
                   the command-line arguments are passed in via
                   args().  Additionally, it is important to note that
                   if key is passed in with a leading "-" then the
                   additional "dash checking" is NOT done. That is, if
                   you call:

<pre>
opts.get_string( "-d", 0.0 );
</pre>

                   then ONLY the entry -d will match, and not --d.
                */
                virtual std::string get_string( const std::string &key, const std::string & defaultVal = std::string() ) const;

                /**
                 * Makes a half-hearted attempt to parse out any args (begining with "-").
                 * Any args without values passed after them are assigned the value true.
                 * Sample valid command lines:
                 *
                   <pre>
                 * foo --a --b foo --c bar --f
                 *   (--a and --f == true, --b == "foo" and --c == "bar")
                 * foo --a eat --b this --c "you lazy bum"
                 *   (--a==eat, --b==this, --c=="you lazy bum")
                 * foo --a=b --c d
                 *   (--a == b, --c == d)
                   </pre>
                 *
                 * These are identical for purposes of get( "c" ):
                   <pre>
                 * [... -c=-1.0 ...] [... -c -1.0 ...] [... --c 1.0 ...] [... --c=1.0 ...]
                   </pre>
                 *
                 *
                 * To get the values, call the property_store API functions like:
                   <pre>
                 *    int foo = parser.getInt( "--i" ); // getInt("i") now works for -i or --i :)
                 *    bool bar = parser.get_bool( "--b" ); // or get_bool( "b") for -b or --b
                   </pre>
                 * 'startat' says which element in argv[] to start with.
                 *
                 * If argpre = 0 then it uses the default argument prefix (defaults to "-"). 
                 * If it is >0 then that is used as a char * prefix
                 * for all arguments.
                 *
                 * This function also stores all processed values in a way familiar to bash and perl users:
                 * $0 = the first argument,
                 * $1 = the second arg
                 * etc.
                 * Thus given:
                   <pre>
                 *     ./foo --arg1 val1 --arg2=foo
                   </pre>
                 * We have:
                   <pre>
                 *  myargs["$1"] == "--arg1"
                 *  myargs["$3"] == "--arg2=foo" (it is arguably useful to split these, but then usage would be
                 *                                inconsistent with bash/perl. However, as it is now it is inconsistent
                 *                                with the results of "--arg2 foo" :/)
                   </pre>
                 * Note that the values are added to this object (or overwrite existing entries), and the list
                 * is not cleared by this function.
                 */
                virtual int args( int argc, char *argv[], int startAt, const char *argpre = "-" );

                /**
                 * Similar to args( int... ) except that this one reads a whole line of options, parses that into
                 * an array, then passes it to parse(...). Note that this _may_ (but should not, ideally) behave slightly
                 * differently from arguments passed to the other form, which typically come in as command-line args
                 * (parsed by your shell). This functions uses a stdstring_tokenizer to do it's parsing, so
                 * any differences in behaviour should be resolved there. i am not aware of any differences.
                 */
                virtual int args( std::string args, std::string separators = " " );

                /**
                   Creates a "usage"-like string for this object containing all keys for which
                   set_help() has been called. If showcurrentvals is true then the current values
                   are also added to th string, otherwise they are left out.

                   Note that the order of the dumped help text/keys is
                   alphabetic (because that's how the container object
                   stores them).
       
                   TODO: maintain the order of args, using the order from set_help().

                   Sample:

                   <pre>
in main() do:
argv_parser & args = argv_parser::args( argc, argv );
if( args.is_hel_set() )
{ // triggers on --help, -help, -? or --?
	cerr << args.dump_help();
        exit( 0 ); // some apps exit with a non-zero for help, some don't.
}
                   </pre>
                */
                virtual const std::string dump_help( bool showcurrentvals = true ) const;

                /**
                   Returns true if -help --help, -? or --? is set.
                */
                virtual bool is_help_set();
        
        private:
                property_store helpmap;
        };
}; // namespace shellish
#endif // shellish_ARGV_PARSER_H
