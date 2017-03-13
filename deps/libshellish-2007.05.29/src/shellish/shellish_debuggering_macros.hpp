#ifndef SHELLISH_DEBUGGERING_MACROS_H
#define SHELLISH_DEBUGGERING_MACROS_H 1

// CERR is a drop-in replacement for std::cerr, but slightly more
// decorative.
#ifndef CERR
#define CERR std::cerr << __FILE__ << ":" << std::dec << __LINE__ << " : "
#define CERRL(A) CERR << A << std::endl;
#endif

#ifndef COUT
#define COUT std::cout << __FILE__ << ":" << std::dec << __LINE__ << " : "
#define COUTL(A) COUT << A << std::endl;
#endif


#endif //  SHELLISH_DEBUGGERING_MACROS_H
