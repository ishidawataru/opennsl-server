%option c++
%{
    /**

    This flexer builds data_node trees out of a subset of XML. Node properties
    are stored/read as XML attributes, and sub-nodes are stored as XML
    elementes. The attribute name "s11n_class" is reserved for internal use.

    XML CDATA, which is not directly supported by the data_node interface,
    is stored in the property named "CDATA", available via
    data_node::get_string("CDATA").
    */

#define YY_SKIP_YYWRAP 1
int yywrap() { return 1; }

#include <stdio.h> // only for EOF definition
#include <cassert>
#include <iostream>
#include <string>
#include <stack>
#include <string>

#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
#define PCERR if( 0 ) CERR

#include <s11n.net/s11n/io/data_node_io.hpp> // node_tree_builder class
#include <s11n.net/s11n/io/data_node_format.hpp> // funxml_serializer class

#include <s11n.net/s11n/io/simplexml_serializer.hpp> // simplexml_sharing_context

#include <s11n.net/s11n/io/strtool.hpp> // translate_entities()

#include <s11n.net/s11n/io/simplexml_data_nodeFlexLexer.hpp>

#define S11N_FACREG_TYPE simplexml_data_nodeFlexLexer
#define S11N_FACREG_TYPE_NAME "simplexml_data_nodeFlexLexer"
#define S11N_FACREG_INTERFACE_TYPE FlexLexer
#include <s11n.net/s11n/factory_reg.hpp>



//////////////////////////////////////////////////////////////////////
// workarounds for the (very outdated) flex output:
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
//////////////////////////////////////////////////////////////////////


namespace
{
        typedef s11n::io::tree_builder_context<
                s11n::io::sharing::simplexml_sharing_context
        > BuilderContext;

        unsigned long bracedepth;
        std::string nodename;
        std::string nodeclass;
        std::string yystr;

        typedef std::stack<std::string> StringStack;
        StringStack cdata;

        void cleanup()
        {
                nodename = "";
                nodeclass = "";
                yystr = "";
                for( StringStack::size_type i = 0; i < cdata.size(); ++i ) cdata.pop();
		bracedepth = 0; // fixed in 1.1.3. W/o this, brace depth can be hosed after reading w/ this lexer multiple times.
                // treebuilder will be freed elsewhere.
        }
}

/** Internal code: note part of the public interface. */
namespace simplexml {

	// duplicated code from funtxt_serializer. TODO: consolidate into one utility function
        bool parseKVP( const std::string & str, std::string & key, std::string & val )
        { // todo: clean this shit up!!!
                PCERR << "parseKVP(["<<str<<"])\n";
                std::string delim = "=";
                std::string::size_type pos;
                pos = str.find_first_of( delim );
                key = str.substr( 0, pos );
                ::s11n::io::strtool::trim_string( key );
                if( std::string::npos == pos )
                {
                        val = "";
                        return true;
                }
                val = str.substr( pos+1 );
                // ::s11n::io::strtool::translate_entities( val, s11n::io::simplexml_serializer_translations(), true );
                PCERR << "prop ["<<key<<"]=["<<val<<"]"<<std::endl;
                return true;
        }

}



int sxml_data_node_close_node( const FlexLexer * context )
{
        // closing something
        if( bracedepth == BuilderContext::node_depth(context) )
        {
                const std::string & cd = cdata.top();
//                 stringutil::trim_string( cd );
                if( ! cd.empty() ) BuilderContext::add_property( context, "CDATA", cd );
                BuilderContext::close_node(context);
        }
        if( 0 == BuilderContext::node_depth(context) )
        {
                // return once we close the first top-level node.
                cleanup();
                return 0;
        }
        cdata.pop();
        --bracedepth;
        return BuilderContext::node_depth(context);
}
%}

SPACE			([ \t])
NONSPACE		([^ \t])
WORD			([_[:alnum:]]+)
WORDS			(({WORD}{SPACE}){2,})
START_OF_LINE		^({SPACE}*)

DIGIT			([0-9])
INTEGER			({DIGIT}+)


DOUBLE_QUOTED_STRING	([\"]([^\"]|(\\\"))+[\"])
SINGLE_QUOTED_STRING	([\'][^\']*[\'])
QUOTED_STRING		({SINGLE_QUOTED_STRING}|{DOUBLE_QUOTED_STRING})

	// QUOTED_STRING: doesn't yet handle escaped quotes-in-quotes. Need to use
	//# separate states for that, i think. 


NUMBER_type1		([-+]?{DIGIT}+\.?([eE][-+]?{DIGIT}+)?)
NUMBER_type2		([-+]?{DIGIT}*\.{DIGIT}+([eE][-+]?{DIGIT}+)?)
NUMBER			({NUMBER_type1}|{NUMBER_type2})

CLASSNAME_CPP_NONS    (({WORD})(<({WORD}([\:,a-zA-Z0-9_<> ])*)>)?)
	// CLASSNAME_CPP_NORMAL	((({WORD}\:\:)+)?({WORD}))
	// CLASSNAME_CPP_TEMPLATE	(([a-zA-Z_])([a-zA-Z0-9_\.\:<>,])*)
CLASSNAME_CPP_TEMPLATE	(({CLASSNAME_CPP_NONS})(\:\:({CLASSNAME_CPP_NONS}))*)
CLASSNAME		({CLASSNAME_CPP_TEMPLATE})
ALMOST_A_WORD		[\._a-zA-Z0-9]+
VARNAME			([a-zA-Z_][_a-zA-Z0-9]*)
VARNAME_LENIENT		([a-zA-Z_][\.\-_a-zA-Z0-9]*|{CLASSNAME})

HEX_DIGIT		([a-fA-F0-9])
RGB_COLOR		(#{HEX_DIGIT}{6})
SEMICOLON		({SPACE}*;+{SPACE}*)

        // ESCAPED_MULTILINE	((.*\$)/[^(\\\n)]*)
        // ESCAPED_MULTILINE	([.]+([^\\]\n$))
        // {ESCAPED_MULTILINE}|
        // PROPERTY_TYPES		({ESCAPED_MULTILINE})
	//UNTIL_SEMICOLON		.+\;{SPACE}*$
	//PROPERTY_VALUE		({NUMBER}|{ALMOST_A_WORD}|{QUOTED_STRING}|{WORD_WITH_PUNCTUATION}|{RGB_COLOR})
        // WORD_WITH_PUNCTUATION	[#.\!\?\-_a-zA-Z0-9]+
        //UNTIL_EOL		([.\n]+[^\\]$)

UNTIL_SEMICOLON		(.+;)

	//ESCAPED_LINES		((.+([\\]\n))+[^\\]\n)
PROPERTY_DECL_RULES	({QUOTED_STRING}|{ALMOST_A_WORD}|{WORDS}|{RGB_COLOR})
PROP_DECL_EQUALS	(({WORD}|{NUMBER}){SPACE}*={SPACE}*)
PROP_DECL_SPACE		(({WORD}|{NUMBER}){SPACE}+)
PROPERTY_DECLS		({PROP_DECL_EQUALS}|{PROP_DECL_SPACE})
PROPERTY_DEFINITION	({PROPERTY_DECLS}{PROPERTY_DECL_RULES})

	// COMMON_DEFINITIONS: See common_flex_definitions.in

KEY_TYPE		({WORD}|{CLASSNAME}|{NUMBER})
			// NUMBER is to help out s11n_node::deserialize_list/Map()

%x IN_ELEM_DECL
%x IN_COMMENT
%x IN_CDATA


%%

	// \<\!.+\n {
	//         continue;
	// 	}

"<?"[^>]+ {;} // todo: add a state to read through these!

"<![CDATA[" {
	BEGIN IN_CDATA;
	}

<IN_CDATA>"]]>" {
        BEGIN 0;
	}

<INITIAL>^({SPACE}+) {continue;}
<INITIAL>({SPACE}+)$ {continue;}

<IN_CDATA>(.|\n) {
        if( ! cdata.empty() ) cdata.top() += YYText();
        continue;
	}

"<"{KEY_TYPE} {
	yystr = YYText();
        //COUT << "opening element? "<<yystr<<std::endl;
        nodename = yystr.substr( 1 );
        BuilderContext::open_node(this, "NoClassYet", nodename);
        cdata.push(std::string());

        ++bracedepth;
        BEGIN IN_ELEM_DECL;
	}

<IN_ELEM_DECL>{KEY_TYPE}=({QUOTED_STRING}|{RGB_COLOR}|{KEY_TYPE}|([a-zA-Z_0-9]+)|(\"\")) {
		std::string key;
		std::string attr;
		if( ! ::simplexml::parseKVP( YYText(), key, attr ) )
                {
		        CERR << "syntax error: could not parse key=value from ["<<YYText()<<"]"<<std::endl;
                        cleanup();
                        return 0;
                }

                if( attr.size() >= 2 )
                { // strip quotes
	                attr.erase(attr.begin());
                        attr.erase(attr.end()-1);
                }
                ::s11n::io::strtool::translate_entities( attr, s11n::io::simplexml_serializer_translations(), true );
                if( "s11n_class" == key  )
                {
                        BuilderContext::change_node_class( this, attr );
                }
                else
                {
                	BuilderContext::add_property( this, key, attr );
                }
                //CERR << "property val=["<<attr<<"] --> ["<<propval<<"]"<<std::endl;
	}

<IN_ELEM_DECL>"/"{SPACE}*">" {
        if( 0 == sxml_data_node_close_node(this) ) return 0;
        BEGIN 0;
	}

<IN_ELEM_DECL>">" {
        // closing element decl.
	//yystr = YYText();
        BEGIN 0;
	}

<IN_ELEM_DECL>"<" {
        CERR << "syntax error: we found a '<' character inside an element declaration." << std::endl;
        cleanup();
        return 0;
        ;
	}

<IN_ELEM_DECL>(.|\n) {;}

\<\/({KEY_TYPE})({SPACE}*)\> {
        if( 0 == sxml_data_node_close_node(this) ) return 0;
	BEGIN 0;
	}

"<!--" {
	//COUT << "entering comment..." << std::endl;
        BEGIN IN_COMMENT;
	}

<IN_COMMENT>"<!--" {
	CERR << "syntax error: you may not have comments within comments." << std::endl;
        cleanup();
	return 0;
        //BEGIN 0;
	}

<IN_COMMENT>"-->" {
	//COUT << "... exiting comment" << std::endl;
        BEGIN 0;
	}
<IN_COMMENT>[.\n] {;}

.|\n {;} // swallow it

%%



#if SIMPLEXML_DO_MAIN
int main( int argc, char ** argv )
{
        using namespace s11n;
        return 0;
}

#endif

