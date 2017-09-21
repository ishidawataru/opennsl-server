%option c++
%{



#define YY_SKIP_YYWRAP 1
int yywrap() { return 1; }


#ifdef NDEBUG
#  undef NDEBUG // force assert()
#endif

#include <stdio.h> // only for EOF definition
#include <iostream>
#include <string>
#include <deque>
#include <cassert>

#include <s11n.net/s11n/io/strtool.hpp> // translate_entities()
#include <s11n.net/s11n/io/data_node_format.hpp> // node_tree_builder, etc.
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR

#include <s11n.net/s11n/io/wesnoth_serializer.hpp>

#define LEXCERR if( 0 ) CERR << "wesnoth lexer: "

#include <s11n.net/s11n/io/wesnoth_data_nodeFlexLexer.hpp>
#define S11N_FACREG_TYPE wesnoth_data_nodeFlexLexer
#define S11N_FACREG_TYPE_NAME "wesnoth_data_nodeFlexLexer"
#define S11N_FACREG_INTERFACE_TYPE FlexLexer
#include <s11n.net/s11n/factory_reg.hpp>


using std::cin;
using std::cerr;
using std::cout;
using std::endl;


// namespace s11n { namespace io {

//         namespace sharing {
                
//         }
// }


namespace wesnoth
{

	// duplicated code from funtxt_serializer. TODO: consolidate into one utility function
        bool parseKVP( const std::string & str, std::string & key, std::string & val )
        { // todo: clean this shit up!!!
                LEXCERR << "parseKVP(["<<str<<"])\n";
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
                LEXCERR << "prop ["<<key<<"]=["<<val<<"]"<<std::endl;
                return true;
        }

}


namespace {
	typedef s11n::io::tree_builder_context<
                s11n::io::sharing::wesnoth_sharing_context
        > BuilderContext;
}
#define METADATA(PROP) BuilderContext::metadata(this).PROP

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

%x READ_PROPERTY_VALUE

%%

({SPACE}*)"#" { continue; /* single-line comment */ }

({SPACE}*)"["({WORD})"="({CLASSNAME})"]" {
        // node opener
        std::string foo = YYText();
        foo = foo.substr(foo.find("[")+1); foo.resize( foo.size()-1 ); // trim braces
        LEXCERR << "Opening node: " << foo << "\n";
	std::string name; // node name
	std::string cname; // class name
        if( ! wesnoth::parseKVP( foo, name, cname ) )
        {
                LEXCERR << "Error parsing out class name from token:\n" << foo << "\n";
                return 0;
        }

        LEXCERR << "class="<<cname<<", name="<<name<<"\n";
        if( ! BuilderContext::open_node( this, cname, name ) )
        {
                LEXCERR << "open_node("<<cname<<","<<name<<") failed. Token="<<foo << std::endl;
                return 0;
        }
        ++METADATA(internaldepth);
        continue;
  }

"[/"({WORD})"]" {
        size_t & bracedepth = METADATA(internaldepth);
        if( bracedepth == BuilderContext::node_depth(this) )
        {
                // avoid closing node when open_node() fails
                LEXCERR << bracedepth-1<<" closing node"<<std::endl;
                BuilderContext::close_node(this);
        }
        --bracedepth;
        size_t nd = BuilderContext::node_depth(this);
        if( 0 == nd )
        {
                // return once we close the first top-level node.
                return 0;
        }
        LEXCERR << "node depth="<<nd<<" brace_depth="<<bracedepth<<std::endl;
  }


<READ_PROPERTY_VALUE>(\\\n)({SPACE})* {;}
<READ_PROPERTY_VALUE>. {
        METADATA(property) += YYText();
	}
<READ_PROPERTY_VALUE>^({SPACE})+ {;} // swallow it
<READ_PROPERTY_VALUE>\n { // end of line
	std::string pname;
	std::string pval;
        if( ! wesnoth::parseKVP( METADATA(property), pname, pval ) ) {
                LEXCERR << "failed parsing key/value pair from property token ["<<METADATA(property)<<"]"<<endl;
                return 0;
        }
	if( (pval.size() >= 2) && ('"' == pval[0]) )
	{ // strip quotes
		pval.erase(pval.begin());
		pval.erase(pval.end()-1);
	}
	::s11n::io::strtool::translate_entities( pval, s11n::io::wesnoth_serializer_translations(), true );
        BuilderContext::add_property( this, pname, pval );
        LEXCERR << "add_property(["<<pname<<"],["<<pval<<"])"<<std::endl;
        BEGIN INITIAL;
  }


({PROP_DECL_EQUALS})(.*) {
        // read property
	if( METADATA(internaldepth) != BuilderContext::node_depth(this) ) { return 1; }
	METADATA(property) = YYText();
        BEGIN READ_PROPERTY_VALUE;
  }


.|\n|{SPACE}+ {;}

%%

#undef METADATA
