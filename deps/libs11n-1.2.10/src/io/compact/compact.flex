%option c++
%{
//
// LICENSE: Public Domain
// Author: stephan - stephan@s11n.net
//

#define YY_SKIP_YYWRAP 1
int yywrap() { return 1; }

// #include <stdio.h>
#include <cassert>
#include <iostream>
#include <string>
#include <deque>
#include <stdio.h> // only for EOF definition
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
#define PCERR if( 0 ) CERR << "compact.flex error:"

#include <s11n.net/s11n/io/strtool.hpp> // hex2int()
#include <s11n.net/s11n/variant.hpp>
#include <s11n.net/s11n/exception.hpp> // io_exception()
#include <s11n.net/s11n/io/data_node_io.hpp> // node_tree_builder
#include <s11n.net/s11n/io/compact_serializer.hpp> // compact_serializer

// #include <s11n.net/cl/cllite.hpp> // classloader framework
#include <s11n.net/s11n/io/compact_data_nodeFlexLexer.hpp>

#define S11N_FACREG_TYPE compact_data_nodeFlexLexer
#define S11N_FACREG_TYPE_NAME "compact_data_nodeFlexLexer"
#define S11N_FACREG_INTERFACE_TYPE FlexLexer
#include <s11n.net/s11n/factory_reg.hpp>

//////////////////////////////////////////////////////////////////////
// workarounds for the (very outdated) flex output:
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
//////////////////////////////////////////////////////////////////////


/**
Basic grammar spec:

{NODE_OPEN}{NAME_SIZE}{NODE_NAME}<class_name_size>{CLASSNAME}
    ({PROP_OPEN}<key_size><key><value_size><value>)*
    (sub-nodes)*
{NODE_CLOSE}

See the lex source for the meanings of the {TOKENS} named above.
*/


namespace {
        unsigned int loops = 0;
        std::string word;
        std::string propname;
        std::string propval;
        std::string nodename;
        std::string nodeclass;
        int decval = 0;
        typedef s11n::io::tree_builder_context<
                s11n::io::sharing::compact_sharing_context
        > BuilderContext;


}



namespace {

        int property_value_size = 4; // number of bytes used to store property values.
        // this changed from 4 to 8 in version 0.5.3.

        //char inchar; // buffer used by READSIZE macro.
}

#define READSIZE(SZ) word = ""; \
        for( int i = 0; i < SZ; i++ )\
        {\
                char inchar = yyinput(); \
                if( 0 == inchar ) {word=""; \
			throw s11n::io_exception("%s:%d: Reached EOF during READSIZE(%d)!",__FILE__,__LINE__,SZ); \
		} \
                word += inchar; \
        } \
	decval = -1; \
	if( ! word.empty() ) decval = ::s11n::io::strtool::hex2int(word); \
	PCERR << "READSIZE("<<SZ<<") decval=="<<std::dec<<decval<<", word= " << word << "\n"; \
	if( decval < 0 ) { \
		throw ::s11n::io_exception("%s:%d: READSIZE(%d) error: error converting word hex number: %s: YYText()==%s",\
__FILE__,__LINE__,SZ,word.c_str(),YYText());	\
	}

#define READVAL(STRINGVAR) \
        for( int lcv = 0; lcv < decval; lcv++ ) { STRINGVAR += yyinput(); } \
	PCERR << "READVAL() decval=="<<decval<<", val=="<<STRINGVAR<<"\n";

%}

HEX_DIGIT		([a-fA-F0-9])
WORD4			({HEX_DIGIT}{4})

	// maintenance note: these hex codes must be kept in sync with those from compact_serializer's enum
NODE_OPEN		f1
NODE_CLOSE		f0
PROP_OPEN		e1
COOKIE4B		51191011
 // ^^^^ cookie for the 4-byte-size-tokens version
DATA_END		51191000

%%

{COOKIE4B} { // s11n 0.5.3 and higher
        property_value_size = 4;
        continue;
	}

{DATA_END} { return 0; }

([ \t\n])+ {;}

{NODE_OPEN} {
        //COUT << "Opening node." << std::endl;
        READSIZE(2); // read node name size
        nodename = "";
        loops = decval;
        READVAL(nodename);
        //cout<< endl;
        READSIZE(2); // get class name size
        nodeclass = "";
        READVAL(nodeclass);
        //COUT << "nodename=["<<nodename<<"]"<<"["<<nodeclass<<"]"<<endl;
        if( ! BuilderContext::open_node( this, nodeclass, nodename ) )
        {
                PCERR<< "open_node("<<nodeclass<<","<<nodename<<") failed." << std::endl;
                //return 0;
		throw ::s11n::io_exception( "%s:%d: open_node(%s,%s) failed",
			__FILE__,__LINE__,nodeclass.c_str(),nodename.c_str() );
        }
	PCERR<< "open_node("<<nodeclass<<","<<nodename<<")" << "depth == " << BuilderContext::node_depth(this) << "\n";
        nodename = nodeclass = "";
	}

{NODE_CLOSE} {
        //COUT << "Closing node." << std::endl;
        BuilderContext::close_node( this );
        if( 0 == BuilderContext::node_depth( this ) )
        {
                // stop once we close the first top-level node.
                return 0;
        }
	PCERR<< "close_node() depth == " << BuilderContext::node_depth(this) << "\n";
        continue;
	}
{PROP_OPEN} {
        //COUTL( "Opening property" );
        propname = "";
        READSIZE(2); // prop name size
        READVAL(propname);
        READSIZE(property_value_size); // get value size
        propval = "";
        READVAL(propval);
        BuilderContext::add_property( this, propname, propval );
	PCERR<< "add property: " << propname << "="<<propval << "\n";
        propval = propname = "";
	}

[.] {
        PCERR << "unexpected token: " << YYText() <<std::endl;
	throw ::s11n::io_exception( "%s:%d: unexpected token: %s", __FILE__,__LINE__, YYText() );
        return 0;
	}

%%

