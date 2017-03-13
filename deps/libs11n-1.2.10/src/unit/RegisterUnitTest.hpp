////////////////////////////////////////////////////////////////////////
// This header should be included once for each UnitTest subclasses
// to register it with the UnitTest classloader. Usage:
// #define UNIT_TEST MyTest
// #define UNIT_TEST_NAME "MyTest"
// #include "RegisterUnitTest.hpp"
////////////////////////////////////////////////////////////////////////

#ifndef UNIT_TEST
#  error "You must define UNIT_TEST before including this header."
#endif

#ifndef UNIT_TEST_NAME
#  error "You must define UNIT_TEST_NAME to the stringified form of UNIT_TEST before including this header."
#endif

#define S11N_FACREG_TYPE UNIT_TEST
#define S11N_FACREG_TYPE_NAME UNIT_TEST_NAME
#define S11N_FACREG_INTERFACE_TYPE UnitTest
#include <s11n.net/s11n/factory_reg.hpp> 
#undef UNIT_TEST
#undef UNIT_TEST_NAME
