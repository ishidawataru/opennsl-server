#ifndef SHELLISH_H_INCLUDED
#define SHELLISH_H_INCLUDED 1

// License: Public Domain
// Author: stephan@s11n.net

#include <vector>
#include <list>
#include <string>
#include <map>
#include <set>
#include <stdlib.h> // size_t
#include <istream>

#include <s11n.net/shellish/shellish_config.hpp>
#include <s11n.net/shellish/environment.hpp>
#include <s11n.net/shellish/aliaser.hpp>
#include <s11n.net/shellish/argv_parser.hpp>
#include <s11n.net/shellish/arguments.hpp>

//#include <s11n.net/shellish/sigslot.hpp>


////////////////////////////////////////////////////////////////////////
// shellish_BUILTIN_PROMPT_FOR_INPUT is the internal command which
// causes shellish to use the current readline handler to fetch
// a line of input. This macro defines the name of the command.
#define shellish_BUILTIN_PROMPT_FOR_INPUT std::string("shellish-prompt-for-input")
// shellish_ENVVAR_READLINE_INPUT defines the name of the environment
// variable used to store the input fetched from a call to 
// shellish_BUILTIN_PROMPT_FOR_INPUT. This is used for internal
// input proxying, is a lame kludge, and will be replaced with a proper
// OO solution. Someday. It is current necessary to allow some client
// code to switch between console-based input and curses, such that
// all built-in eshell handlers can receive/send/ i/o from both modes
// with no special concerns. While the support is cool, the implementation
// itself is "quite unfortuante."
#define shellish_ENVVAR_READLINE_INPUT std::string("SHELLISH_READLINE_INPUT")


/**
   shellish is an API encapsulating some Unix-shell-like behaviours
   for a shell-like app framework. It's main goal is to simplify the
   creation of console-based applications by providing features
   commonly needed by such apps, in particular command input and
   dispatching.

   It is NOT a full-fledged shell, and does not aim to become
   one. Ever!  It has, instead, evolved from a small framework over
   the years (since early 2001) to provide a simple solution for
   plugging in "command-drive" applications to a shellish console
   interface. This has proven useful for writing interactive library
   test code and as a general-purpose console-UI interface for a
   number of test, prototype, and "less fancy" applications.

   While it needs to be completely rewritten and restructured, it
   currently provides the following services en masse:

   - Console input. If available, it uses libreadline_cpp, which in
   turn may delegate to other libraries, like GNU Readline. By default
   it uses plain old cin, which isn't much fun for interactive mode.

   - Provides a global dispatcher which maps single-token "command
   names" to functions which handle such commands. This library parses
   user input in much the same way which most shell interfaces would
   (minus any control structures), which is to say that command
   handlers get a list of string arguments, much in the same way
   that a main() function gets a list of arguments. Likewise, they
   returns 0 on success and non-zero on error.

   - Provides the most basic shell-like features, including $variables
   and the most common shell commands (cd, pwd, echo, sleep, ...).

   - If your platform supports it (has libdl or liblt_dl, or is Win32),
   rudimentary plugins support allows users to open arbitrary DLLs at
   runtime. This can be used to load shellish plugins or client-side
   DLLs which use the same classloader model as this code (documented
   at length on the s11n.net site).

   - If libs11n support (1.1.3+) is enabled then session files are
   supported which make the aliases, command history (if available),
   and optionally the environment variables, persistent across app
   sessions.


   Brief History:
   
   shellish derives from a utility library named 'eshell', which
   supported more or less the same features but which was structurally
   a bit different, requiring more dependencies on third-party/add-on
   code. This library is mainly a refactoring of eshell to make it easier
   to get running on various systems, minus the various support libraries
   which used to be required dependencies.

*/
namespace shellish {

	std::string shellish_version();

        /**
           CommandErrors defines a common set of return codes which
           "should" be used by command handlers.
        */
        enum CommandErrors {
        ErrorNoArguments = -1,
        NoError = 0,
        EShellExit = 0, // triggered by commands quit/exit/logout
        CommentLine = 0,
        Error = 1,
        ErrorUsageError = 2,
        ErrorParseError = 3,
        ErrorResourceAcquisition = 126, // e.g., file open error.
        ErrorResourceNotFound = 127,
        ErrorCommandNotFound = 128,
        ErrorCommandNotMapped = 129,
        ErrorWTF = 255,
	ErrorException = 666 // exception was internally caught
        };

        /**
           Clients should call this from main(), passing it the same
           arguments main() receives. argv is never modified, but is
           non-const for historical reasons.

           Returns the same as args() - an object populated with
           the arguments from argv.

           This function recognizes the following command-line
           arguments:

           --shellish-debug: enables internal debugging.

	   --shellish-classpath: colon-delimited path of dirs to use
	   for plugin lookups. Used by the 'dlload' command, for
	   example.

           This function calls setup_default_handlers().

        */
        argv_parser & init( int argc, char ** argv );


        /**
           Signature for command-handling methods.

           Clients map command_handler_funcs to commands using
           map_commander().

           Note that on 25 Sept 2004 the second argument (std::ostream
           &) was removed from this interface! Use eshell::ostream()
           instead, if you need to send output to a stream. Some
           clients hook that stream in to their UI widgets and
           curses-mode windows.
        */
        typedef int (*command_handler_func)( const arguments & );


        typedef std::map<std::string,command_handler_func> commander_map;


        /**
           Dispatcher holds a map of commands to command handler
           functions. Clients may plug their own dispatcher instances
           in as the default handler to be used by dispatch(),
           map_commander(), etc.
        */
        class dispatcher
        {
        public:
                dispatcher(){}
                virtual ~dispatcher() {}

                /**
                   Returns ErrorCommandNotMapped if args[0] is not
                   mapped, otherwise it ruturns whatever the
                   registered handler returns (which might also be
                   ErrorCommandNotMapped).

                   As a special case, if args[0] is not mapped, this
                   implementation searches for a handler mapped to
                   "*", the catch-all-handler. If one is found it is
                   used. Subclasses are not required to implement this
                   behaviour.

                   By convention, handlers return 0 on success.
                */
                virtual int dispatch( const arguments & args );

                commander_map & handler_map()
                {
                        return this->m_map;
                }

                const commander_map & handler_map() const
                {
                        return this->m_map;
                }

                void map( const std::string & key, command_handler_func func )
                {
                        this->handler_map()[key] = func;
                }

        private:
                commander_map m_map;
        };


//         /**
//            EXPERIMENTAL!
//         */
//         template <typename Context>
//         commander_map & handler_map()
//         {
//                 static commander_map bob;
//                 return bob;
//         }

        /**
           Returns the current dispatcher. See
           dispatcher(dispatcher*).
        */
        dispatcher & default_dispatcher();

        /**
           Sets the current dispatcher used by map_commander(),
           dispatch(), etc.

           Passing it zero will revert to the default dispatcher.

           Ownership does not change by calling this: if the caller
           owns the dispatcher before calling this, he still owns it
           afterwards.
        */
        void default_dispatcher( dispatcher * );

        typedef std::map<std::string,dispatcher *> dispatcher_map;

        /**
           Not yet used.

           Returns a map of available named dispatchers, including
           the built-in default dispatcher.

           This support is intended to obviate the default dispatcher
           at some point.
        */
        dispatcher_map & dispatchers();


        /**
           Gets a named dispatcher from dispatchers(). Returns 0 if no
           dispatcher with that name has been added via
           add_dispatcher(). 

           Ownership does not change by calling this: if the caller
           owns the dispatcher before calling this, he still owns it
           afterwards.
        */
        dispatcher * get_dispatcher( const std::string & name );

        /**
           Adds a named dispatcher to dispatchers().

           Ownership does not change by calling this: if the caller
           owns the dispatcher before calling this, he still owns it
           afterwards.
        */
        void add_dispatcher( const std::string & name, dispatcher * dist );



        /**
           Returns this shell's environment.

           See the docs for environment::propagate_sets(): that is set
           to true in the returned object.
        */
        environment & env();

        /**
           Returns the arguments passed in to shellish::init(). The returned
           object is not valid before shellish::init() is called.
        */
        argv_parser & args();

        /**
           Returns this shell's aliases
        */
        aliaser & aliases();

        /**
           Sets the default ostream used by ostream(). Ownership of
           the stream does not change. If 0 is passed then std::cout
           will be used for further calls to ostream().

	   curses-based clients can use streambuffer redirection to
	   tie all cout/cerr output to arbitrary curses widgets.
        */
        void ostream( std::ostream * );

        /**
           Returns the default ostream used by command handlers
	   which need to stream to the 'logical stdout'.

	   Calling this function ostream was a bad idea, but sed'ing
	   it out at this point would be problematic.
         */
        std::ostream & ostream();

	/** See ostream(). This is the same, but intended to be used
	    like cerr. */
        void errstream( std::ostream * );
	/** See ostream(). This is the same, but intended to be used
	    like cerr. */
        std::ostream & errstream();

        /**
	   Expands aliases and variables in command and calls
           calls dispatch( arguments(expanded_cmd) ).

	   Be very aware of the expansion policy used here,
	   as it will behave differently than calling the
	   nearly-equivalent code of:

	   dispatch( arguments( command ) );

	   which does no expansions of any sort.
        */
        int dispatch( const std::string & command );

        /**
           Passes off the arguments to the command_handler_func assigned to args[0].
           Returns zero on success, non-zero on error. Note that args[0] is
           retained, and passed on to the handler.

           The most recent return value is available via:
           env() as the property "?". Thus 'echo $?' will echo the last
           return value.

           args contains the input arguments.

         */
        int dispatch( const arguments & args );

        /**
           For each line of input in the given istream
           dispatch(line) is called.

           Returns the error code of the last command.
	   If stopOnError is true then it stops processing
	   if a command returns non-zero.

	   Empty lines and comment lines (starting with "#") are
	   ignored.
        */
        int dispatch( std::istream & is, bool stopOnError = false );

	/**
	   Expands ~ or ~/ at the start of a token to $HOME. If the
	   single token "~username" is passed in then /etc/passwd info
	   is used (if possible) to expand the token.

	   If it returns false, token was not modified, otherwise it
	   was.
	*/
	bool expand_tilde_home_inline( std::string & token );

	/**
	   Like the non-const variant, but returns the expanded string
	   without modifying the input.
	*/
	std::string expand_tilde_home( const std::string & token );

        /**
           Maps a single-token command to a command_handler_func.
           
           As a special case, the key "*" is considered to be a
           wildcard-handler, and is used if no handler can be found
           for a given key. By default there is no wildcard handler.

           If helptext is non-empty then map_help(key,helptext) is
           called.
        */
        void map_commander( const std::string & key, command_handler_func, const std::string & helptext = "" );

        /**
           Maps help text for command 'key'.

           This function will probably go away someday, and moved into
           an externally-handled help subsystem.
        */
        void map_help( const std::string & key, const std::string & text );

        /**
           Reads a line of input and returns it. The exact mechanism
           used to read the input depends on whether or not this lib
           was build with libreadline_cpp support.

           When this function returns, if breakout is true then this
           is a signal to the client that no more input should be
           read. This is normally triggered by Ctrl-D, but that
           depends on the exact input method used by this library (see
           below).

           Note that $var expansion is done on prompt, so it may
           containe environment variables. Also, the input line is
           added to the command-line history if this library is built
           with libreadline_cpp support.

           No other processing is done: the exact input value is
           passed back to the client without shellish dispatching it or
           setting any error codes.

           If shellish is build with readlinecpp::Readline then the
           input is collected by whatever mechanism
           readlinecpp::Readline supports. If it is not built with
           this support then std::cin is used for input.

        */
        std::string read_line( const std::string & prompt, bool & breakout );

	/**
	   Similar to read_line(), but internally works quite differently:

	   This function calls dispatch( "shellish-prompt-for-input "+prompt )
	   to collect input, which means that clients may swap out the
	   user input routine by remapping that handler to their own.

	   The shellish-prompt-for-input handler should do the following:

	   a) Treat $1 as an input prompt.

	   b) Accept one line of input from the user. Normally this means
	   one COMMAND, but if used in conjunction with input_loop(),
	   input_loop() will accept semicolon-separated commands.

	   c) If the input is not empty, call:

	   shellish::env().set( shellish_ENVVAR_READLINE_INPUT, theinput );

	   if the input is empty, call:

	   shellish::env().unset( shellish_ENVVAR_READLINE_INPUT );

	   d) Then the handler should return 0. There are no common
	   error conditions which can be handled by this function if
	   non-zero is returned.

	   If you implement a custom handler which is intended to
	   interact properly with input_loop(), the handler should
	   call end_input_loop() if the handler detects that the
	   "absolute end" of input is reached, and that shellish should
	   stop taking input. This would normally be the desired
	   behaviour, e.g., when the user enters Ctrl-D.


	   BUG: the prompt string must not have any double-quotes in
	   it.
	*/
	std::string prompt_for_input( const std::string & prompt );

	/**
	   This is the default handler used by prompt_for_input().
	   Clients which remap shellish-prompt-for-input may want to
	   restore the original mapping at some point, and should map
	   it to this function.

	   This implementation calls read_line(), and calls
	   end_input_loop() if read_line() says that input should end.
	   As prompt_for_input() specifies, this function stores it's
	   result string in env()[shellish_ENVVAR_READLINE_INPUT], or
	   unsets it if the input line is empty.

	   args[0] is ignored, but is expected to be
	   "shellish-prompt-for-input" (or the command which triggered
	   this handler).

	   args[1] should be a prompt string. If none is set then
	   env()["SHELLISH_PROMPT"] is used, and ">" is used if no other
	   option is available.

	   This function always returns 0.
	*/
        int handle_prompt_for_input( const shellish::arguments & args );

        /**
           Accepts input from the user, line-wise, potentially
           forever. It dispatch()es each input line. The input is
           tokenized, using ';' as a separation character, so multiple
           inputs can be given at once using the form:

           command_one args; command_two args; ...


	   This function gets its input by calling:

	   prompt_for_input( prompt )

	   Please see that function for details of how to plug your
	   own input handler via this technique.

	   Each command is sent via dispatch(string), so that function's
	   $variable/alias expansion policy applies.

           The given prompt is used as an input prompt. The prompt
	   will be expanded using expand(prompt) before it is shown,
	   and thus it may reference variables which are updated
	   between commands.

	   This function returns when a command handler has called
	   end_input_loop(). In stdin/readline/editline modes this
	   happens when Ctrl-D is pressed.

	   This function is NOT reentrant: never call it recursively!
	   This means that this function is OFF LIMITS to ANY command
	   handlers, as they might have been dispatched through this
	   function. It is only for use at the top-most client level.

	   This function does not yet gracefully recover from
	   exceptions: it might be left in a useless state
	   if one propagates out.
        */
        void input_loop( const std::string & prompt );

        /**
           Triggers input_loop() to exit. This can be used by
           command_handler_funcs to, e.g., accept a "quit" command as
           an alias for Ctrl-D. The handler should call this once to
           tell input_loop() to stop accepting input. In practice
	   it is never necessary to call this directly unless you
	   reimplement the default quit/exit commands.
         */
        void end_input_loop();

        /**
           This function sets up several default commander handlers.

           - quit, exit, logout: act as Ctrl-D.

           Shell-like commands:

           - echo: prints it's arguments to stdout.

           - set/unset: sets/unsets env vars.

           - env: lists the current environment vars.

           - cd/chdir: changes current working directory.

           - pwd: prints current working directory.

           - alias: aliases one command to another.

           - help: reserved, but not yet implemented

           - sleep: sleeps for X seconds. Pass anything as a second
           argument to make it verbose.

           - which: finds file arg[1] in arg[2] (which defaults to
           $PATH).

           - history: lists command line history (if that is enabled).


           Launching processes:

           - fork: run arguments using fork()/exec(). Use this for X11 apps.

           - system, !: run arguments using system(). Use this for
           screen-mode apps (vi, console-mode emacs, etc).

           - pipe: run arguments using popen(). Use this for
           stream-oriented apps (nearly any command which simply
           generates output to stdout).

           - bgthread (only if shellish is built with pthreads
           support): runs the arguments in a background thread. It
           cannot be recovered to the foreground.

           General reminder: shellish has no job control of any sort.

           Misc:

           - shellish-debug-toggle: toggles internal shellish debugging
           output.

           - dlload: opens a DLL.

           Clients may load their own libraries via this function by
           setting the environment variable SHELLISH_AUTOLOAD_LIBS to
           the library names, in the following format:

           libfoo:libbar:/absolute/path/also/okay/for/libwhatever.so

           If you set the environment var SHELLISH_AUTOLOAD_LIBS
           before calling init() then the compile-time default
           autoload list (which may be empty) is not used, otherwise
           it is.
        */
        void setup_default_handlers();

        /**
	   Expands env() vars in the input string. It does NOT do
	   aliases() or tilde-home expansion, because these
	   historically are problematic here. For example, aliases are
	   only expanded in the first token of a command, but this
	   function is intended to be useful for individual tokens as
	   well as whole lines. The tilde-home processing was removed
	   because it was found that the tilde key was sometimes
	   useful/necessary as-is (unparsed) for client apps. Same
	   for asterisks and question marks, both of which are normally
	   wildcard characters.

	   See expand_tilde_home() for the tilde-home expansion and
	   aliases().expand() for alias expansion.

	   Wildcard expansion was removed, first because the '*' and
	   '?' became necessary in a client app of mine, and using
	   ::wordexp() turned out to be problematic and kludgy.
        */
        std::string expand( const std::string & );


	/**
	   Helper function. Returns true only if '#' is the
	   first non-space character in str. For empty lines
	   it returns false.
	*/
        bool is_comment_line( const std::string & str );


        /**
           EXPERIMENTAL!
        */
        namespace ui
        {


                /**
                   EXPERIMENTAL!
                */
                struct menu_entry
                {
                        typedef ::shellish::command_handler_func func_type;
                        menu_entry( const std::string & _name,
                                    func_type _func,
                                    const ::shellish::arguments & _func_args = ::shellish::arguments()
                                    ) : name(_name), func(_func), func_args(_func_args) {}
                        menu_entry() : name(""),func(0), func_args("") {}

                        /**
                           Label for this menu entry.
                        */
                        std::string name;

                        /**
                           Callback function for this menu entry.
                        */
                        func_type func;

                        /**
                           Hint to client specifiying arguments to
                           pass on to func() when/if it is called.
                        */
                        ::shellish::arguments func_args;
                };

                /**
                   EXPERIMENTAL!
                */
                struct menu
                {
                        typedef std::list< menu_entry > menu_entry_list;
                        typedef menu_entry::func_type func_type;
                        explicit menu( const std::string & _name ) : name(_name) {}
                        menu() {}
                        std::string name;
                        menu_entry_list entries;
                        inline void add( const std::string & label,
                                         func_type func,
                                         const ::shellish::arguments & func_args = ::shellish::arguments() )
                        {
                                this->entries.push_back( menu_entry( label, func, func_args ) );
                        }
                };

                /**
                   EXPERIMENTAL!
                */
                int process_menu( const menu & themenu,
                                  bool loop = true );


                /**
                   Very rudimentary progress meter.

                   Spits out a "." every INTERVAL ticks.

                   Sample usage:

<pre>
shellish::ui::simple_progress prog(3,'.');
for( int i =0 ; i < 15; i++ ) ++prog;
</pre>

would send a '.' character to shellish::ostream() every 3 ticks.
                */
                class simple_progress
                {
                public:
                        /**
                           Creates a progress meter with the given
                           ostream, tick_interval and tick character.
                        */
                        inline simple_progress( size_t tick_interval = 5,
                                                char tickchar = '.' )
                                : m_tick(0),
                                  m_interval(tick_interval), m_tchar(tickchar)
                        {
                        }

                        /**
			   Adds one to the tick count and sends a
			   character to shellish::ostream() if
			   (ticks() % tick_interval) == 0.
                        */
                        inline void tick()
                        {
                                if( 0 == ( this->m_tick++ % m_interval ) )
                                {
                                        ::shellish::ostream() << this->m_tchar;
                                }
                        }

                        /**
                           Returns the number of times tick() has been called.
                         */
                        inline size_t ticks() const
                        {
                                return this->m_tick;
                        }

                        /**
                           Same as calling tick().
                        */
                        inline void operator++() // prefix
                        {
                                this->tick();
                        }

                private:
                        size_t m_tick;
                        size_t m_interval;
                        char m_tchar;
                }; // simple_progress

        } // namespace ui

} // namespace




#endif // SHELLISH_H_INCLUDED
