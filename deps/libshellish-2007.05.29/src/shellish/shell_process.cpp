#include <stdio.h> // popen()
#include <stdlib.h> // system()
#include <sys/types.h> // pid_t
#include <sys/wait.h> // pid_t
#include <unistd.h> // fork()

#include <vector>

#include "shell_process.hpp"
#include "shellish_debuggering_macros.hpp" // COUT/CERR
#include <s11n.net/shellish/stdstring_tokenizer.hpp>

#define ELIB_USE_SIGNALS 1
#if ELIB_USE_SIGNALS
#  include <stdlib.h> // atexit()
#  include <signal.h> // signal()
#  include <setjmp.h> // longjmp()
#endif


namespace shellish {
        using std::string;
        using std::endl;

#if ELIB_USE_SIGNALS
        sigjmp_buf shell_process_ctrl_c_jmp_buffer;
        void shell_process_SIGINT(int)
        {
                std::cout << "^C" << std::endl; // this really shouldn't be here, but i find it useful.
                siglongjmp( shell_process_ctrl_c_jmp_buffer, 1 );
        }
#endif

        shell_process::shell_process()
        {
                static bool donesetup = false;
                if( !donesetup && (donesetup=true) )
                {
                        shell_process::setupSigchldIgnore();
                }
        }

        shell_process::~shell_process()
        {
        }


        void
        shell_process::setupSigchldIgnore()
        {
#if ELIB_USE_SIGNALS
                // i honestly have no clue what the following signal-handling
                // code does, exactly, except that it gets rid of defunct
                // child processes.
                // http://www.erlenstar.demon.co.uk/unix/faq_8.html#SEC83
                struct sigaction sa;
                sa.sa_handler = SIG_IGN;
#ifdef SA_NOCLDWAIT
                sa.sa_flags = SA_NOCLDWAIT;
#else
                sa.sa_flags = 0;
#endif
                sigemptyset(&sa.sa_mask);
                sigaction(SIGCHLD, &sa, 0);
#endif // if ELIB_USE_SIGNALS

        }

        int
        shell_process::system( const std::string &cmdline, std::ostream & os  )
        {
                //LIBE_VERBOSE << "system()ing ["<<cmdline<<"]"<<std::endl;
                return ::system( cmdline.c_str() ) / 256;
        }

        int
        shell_process::fork( const std::string &cmdline, std::ostream & os  )
        { // my very first fork()/exec()
                if( cmdline.empty() )
                {
                        os << "shell_process::fork(): empty command passed. :(" << std::endl;
                        return 1;
                }
                //CERR << "fork()ing ["<<cmdline<<"]"<<std::endl;
                pid_t pid;
                pid = ::fork();
                if( pid == -1 )
                {
                        os << "shell_process::fork("<<cmdline<<"): error fork()ing." << endl;
                        return 1;
                }
                if( pid > 0 ) return 0; // parent, go away.

                // child process...

                strtool::stdstring_tokenizer tokens( cmdline, " \n\t" );

                typedef std::vector<char *> VecT;
                VecT args;
                int i = 0;
                string tok;
                string filearg;
                for( ; tokens.has_tokens(); i++ )
                {
                        tok = tokens.next_token();
                        if( i == 0 ) filearg = tok;
                        if( tok.empty() ) continue;
                        //os << "adding token ["<<tok<<"]"<<endl;
                        args.push_back( const_cast<char *>( tok.c_str() ) );
                        // this ^^^^^^^^^^^ const_cast is theoretically okay
                        // because we're not gonna de-allocate the
                        // array and this process will never pass
                        // control back to anyone else, so tokens
                        // shouldn't be unduly molested behind our
                        // back. i won't swear that it's safe, though :/.
                }
                args.push_back( (char *)0 );
                int ex = execvp( filearg.c_str(), &args[0] ); // this should never return.
                CERR << "execvp() returned " << ex << ". That means it failed. :(" << std::endl;
                _exit(ex);
                return ex; // avoid potential "no return statement" warnings.
         }

        int
        shell_process::pipe( const std::string &cmdline, std::ostream & os )
        {
                if( cmdline.empty() )
                {
                        os << "shellish::shell_process::pipe(): cannot run an empty command :(";
                        return 1;
                }
                //LIBE_VERBOSE << "pipe()ing ["<<cmdline<<"]"<<std::endl;
                FILE *fp = 0;
                fp = popen( cmdline.c_str(), "r" );
                if( ! fp )
                {
                        os << "shell_process::pipe(): popen("<<cmdline<<") failed :(" << std::endl;
                        return 1;
                }
//                 fwrite( cmdline.c_str(), sizeof( char ), cmdline.size(), fp );
#if ELIB_USE_SIGNALS
#define RESTORE_SIGNALS signal( SIGINT, old_sighandler )
                typedef void (*signalfunc)(int);
                signalfunc old_sighandler = signal( SIGINT, shell_process_SIGINT );
                if( 0 == sigsetjmp( shell_process_ctrl_c_jmp_buffer, 1 ) )
                {
#endif
			int getcchar;
                        while( ! feof( fp ) )
                        {
                                getcchar = getc( fp );
                                if( getcchar == EOF ) break;
                                os <<(unsigned char) getcchar;
                                if( ! os.good() )
                                {
#if ELIB_USE_SIGNALS
                                        RESTORE_SIGNALS;
#endif
                                        CERR << "shell_process::pipe(): error in output stream!" << std::endl;
                                        return 1;
                                }
                        }
#if ELIB_USE_SIGNALS
                }
                RESTORE_SIGNALS;
#endif
                int ret = pclose( fp ); // this is always -1 on my boxes :(
                // CERR << "pipe("<<cmdline<<") returning " << std::hex << ret << std::endl;
                return ret; 
        }


#undef ELIB_USE_SIGNALS
#undef RESTORE_SIGNALS

} // namespace shellish
