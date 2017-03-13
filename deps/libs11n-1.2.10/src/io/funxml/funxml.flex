%option c++
%{
#define YY_SKIP_YYWRAP 1
int yywrap() { return 1; }

#include <stdio.h> // only for EOF definition
#include <cassert>
#include <iostream>
#include <string>
#include <deque>

#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
#define PCERR if( 0 ) CERR

#include <s11n.net/s11n/io/strtool.hpp> // translate_entities()
#include <s11n.net/s11n/io/data_node_format.hpp> // node_tree_builder, tree_builder_context
#include <s11n.net/s11n/io/funxml_serializer.hpp> // funxml_sharing_context


#include <s11n.net/s11n/io/funxml_data_nodeFlexLexer.hpp>

#define S11N_FACREG_TYPE funxml_data_nodeFlexLexer
#define S11N_FACREG_TYPE_NAME "funxml_data_nodeFlexLexer"
#define S11N_FACREG_INTERFACE_TYPE FlexLexer
#include <s11n.net/s11n/factory_reg.hpp>



//////////////////////////////////////////////////////////////////////
// workarounds for the (very outdated) flex output:
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
//////////////////////////////////////////////////////////////////////


namespace // funxml internal
{
        // a communication context to talk to s11n with.
        typedef s11n::io::tree_builder_context<
                s11n::io::sharing::funxml_sharing_context
        > BuilderContext;

}

////////////////////////////////////////////////////////////////////////////////
// Get instance-specific vars, stored externally because
// we can't change the FlexLexer-gen'd class to add them
// as members.
#define METADATA(PROP) BuilderContext::metadata(this).PROP
////////////////////////////////////////////////////////////////////////////////

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

	// COMMON_DEFINITIONS: See common_flex_definitions.at

KEY_TYPE		({WORD}|{NUMBER})
			// NUMBER support is to help out s11n_node::deserialize_list/Map()

%%



\<{KEY_TYPE}{SPACE}+"class="({QUOTED_STRING}){SPACE}*\> {  // QUOTED_STRING is to support class templates, which have translated entities.
	// opening a node
        ++METADATA(internaldepth);
        std::string & yy = METADATA(bufferyy);
        yy = YYText();
        PCERR << "? opening class node ? ["<<yy<<"]" << std::endl;

        std::string::size_type opos = yy.find( "class=\"" ) + 7;
        std::string::size_type cpos = yy.find( "\"", opos );

        std::string & nodename = METADATA(nodename);
        std::string & nodeclass = METADATA(nodeclass);
        nodeclass = yy.substr( opos, cpos - opos );
        nodename = yy.substr( 1, yy.find_first_of( " \t\n" ) - 1 );
        ::s11n::io::strtool::translate_entities( nodeclass, s11n::io::funxml_serializer_translations(), true ); // handle class templates
        BuilderContext::open_node(this, nodeclass, nodename);
        PCERR << "opening node ["<<nodeclass<<"] impl_class=["<<nodename<<"]\n";
        continue;
	}

\<{KEY_TYPE}\> {
        // opening a property
        METADATA(property) = "";
        ++METADATA(internaldepth);
	}

\<\/{KEY_TYPE}\> {
        // closing something
        std::string & yy = METADATA(bufferyy);
        std::string & cdata = METADATA(property);
	yy = YYText();
        size_t & depth = METADATA(internaldepth);
        if( depth != BuilderContext::node_depth(this) )
        { // closing a property.
                std::string prop = yy.substr( 2, yy.size() - 3 );
                ::s11n::io::strtool::translate_entities( cdata, s11n::io::funxml_serializer_translations(), true );
                PCERR << "adding property ["<<prop<<"]=["<<cdata<<"]\n";
                BuilderContext::add_property( this, prop, cdata );
        }
        else
        { // closing an object node.
                PCERR << "closing node. ["<<METADATA(nodename)<<"]\n";
                BuilderContext::close_node(this);
        }
        --depth;
        cdata = "";
        if( 0 == depth )
        {
                PCERR << "closing root node.\n";
                // return once we close the first top-level node.
                return 0;
        }


   }



.|\n|({SPACE}) { METADATA(property) += YYText();}

%%

#undef METADATA
