%option c++
%{
/**
LICENSE: Public Domain
Author: stephan - stephan@s11n.net

This lexer reads in a lisp-like (but not lisp) grammar for the s11n
framework. It's output partner is s11n::paren_serializer.

Sample:

nodename=(ImplClassName (propery_name property value) (prop2 value of \) prop2) 
	another_node=(ns::ClassName)
)

nodename represents an s11n node name. ImplClassName represents the
object's implementation class name.

Note that closing parens in your data must be backslash-escaped. This
parser arguably strips all non-paired backslashes, so any actual
backslashes must also be escaped (C-style). The parens_serializer takes
this into account and escapes it's serialized data.

On parse errors it throws a std::runtime_exception.

*/

#define YY_SKIP_YYWRAP 1
int yywrap() { return 1; }

#include <stdio.h> // only for EOF definition
#include <iostream>
#include <string>
#include <stdexcept>

//////////////////////////////////////////////////////////////////////
// workarounds for the (very outdated) flex output:
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
//////////////////////////////////////////////////////////////////////


#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR
#define PCERR if( 0 ) CERR << "parens lexer: " 

#include <s11n.net/s11n/io/strtool.hpp> // translate_entities()
#include <s11n.net/s11n/io/data_node_io.hpp> // node_tree_builder
#include <s11n.net/s11n/io/parens_serializer.hpp> // parens_serializer

#include <s11n.net/s11n/io/parens_data_nodeFlexLexer.hpp>
#define S11N_FACREG_TYPE parens_data_nodeFlexLexer
#define S11N_FACREG_TYPE_NAME "parens_data_nodeFlexLexer"
#define S11N_FACREG_INTERFACE_TYPE FlexLexer
#include <s11n.net/s11n/factory_reg.hpp>

namespace  {

        typedef s11n::io::tree_builder_context<
                s11n::io::sharing::parens_sharing_context
        > BuilderContext;
}

#define METADATA(PROP) (BuilderContext::metadata(this).PROP)

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


CLASSNAME_PARENS (([A-Za-z])|({QUOTED_STRING})|({CLASSNAME}))

%x OPEN_CLASS
%x IN_PROPERTY
%x IN_COMMENT_BLOCK
OPENER		\(
CLOSER		\)
NODENAME 	{VARNAME}
PROPERTY	{VARNAME}

	// experimental:
METADATA_KEY	"parens:metadata"
%x IN_METADATA

%%

"(*" { // (* comment blocks *)
	BEGIN IN_COMMENT_BLOCK;
	}

<IN_COMMENT_BLOCK>EOF {
        PCERR << "hit EOF in a (*comment block*)." << std::endl;
        throw std::runtime_error( "hit EOF in a (*comment block*)" );
        return 0;
	}
<IN_COMMENT_BLOCK>([^\\])?"*)" { /* this match isn't 100% correct.*/ BEGIN 0; }
<IN_COMMENT_BLOCK>({SPACE}|\n)+ { continue;}
<IN_COMMENT_BLOCK>. { continue;}

({SPACE})*[#;].*$ {;} // comment lines



{OPENER}({SPACE})* {
        BEGIN IN_PROPERTY;
	}

<IN_PROPERTY>(({VARNAME})|({NUMBER}))({SPACE})* { // key name of property
        std::string propname = YYText();

        // strip leading/trailing spaces from the property name:
        static const std::string avoid = " \t\n";
        std::string::size_type tail = propname.find_last_not_of( avoid );
        propname = propname.substr( propname.find_first_not_of( avoid ),
                                    (std::string::npos == tail) ? tail : (tail + 1) );
        PCERR << "reading property ["<<propname<<"]...";

        // Now we consider all data until a non-escaped closing brace
        // to be the value of the property...
        std::string propval = "";
        unsigned char c = yyinput();
        bool escaped = false;
        while( 0 != c )
        {
                if( (!escaped) && ('\\' == c) )
                { // next char will be considered escaped
                        escaped = true;
                        propval += c;
                        c = yyinput();
                        continue;
                }
                if ( (!escaped) && (')' == c) )
                {
                        // Look for a non-escaped paren to close us.
                        break;
                }
                escaped = false;
                propval += c;
                c = yyinput();
                //PCERR << "["<<c<<"]\n";
        }
        if( 0 == c )
        {
                std::string errmsg = "Reached EOF while reading value for property '" + propname + "'. This shouldn't happen.";
                PCERR << errmsg << std::endl;
                throw std::runtime_error( errmsg );
                return 0;
        }

        ::s11n::io::strtool::translate_entities( propval, s11n::io::parens_serializer_translations(), true );
        //std::cout << "["<<propval<<"]"<<std::endl;
        PCERR << "Setting property: ["<<propname<<"]=["<<propval<<"]\n";
        BuilderContext::add_property( this, propname, propval );
        BEGIN 0;
	}

{NODENAME}={OPENER} {
        std::string tmpstr = YYText();
        METADATA(nodename) = tmpstr.substr( 0, tmpstr.find( "=" ) );
        PCERR << "opener: node name="<<METADATA(nodename)<<std::endl;
        BEGIN OPEN_CLASS;
	}


<OPEN_CLASS>({SPACE})+ {;}

<OPEN_CLASS>{CLOSER} { // special case: empty node:
        --METADATA(internaldepth);
        BEGIN 0;
	}
<OPEN_CLASS>({SPACE}|\n)+ {;}

<OPEN_CLASS>{CLASSNAME_PARENS} {
        std::string & nodename = METADATA(nodename);
        std::string & nodeclass = METADATA(nodeclass);
        nodeclass = std::string(YYText());
        if( nodeclass.empty() )
        {
                std::string errmsg = "nodes may not have empty names!";
                PCERR << errmsg <<std::endl;
                throw std::runtime_error( errmsg );
                return 0;
        }
        if( ('\"' == nodeclass[0]) || ('\'' == nodeclass[0]) )
        { // remove quotes from class name
                nodeclass.erase(0,1);
                nodeclass.resize(nodeclass.size()-1); // chop trailing quote
        }

        PCERR << "opening '"<<nodename<<"', class=" << nodeclass << std::endl;
        ++METADATA(internaldepth);
        if( ! BuilderContext::open_node( this, nodeclass, nodename ) )
        {
                std::string errmsg = "open_node(" + nodeclass + "," + nodename + ") failed.";
                PCERR<< errmsg << std::endl;
                throw std::runtime_error( errmsg );
                return 0;
        }
        BEGIN 0;
	}

<OPEN_CLASS>[. \t\n] {
        std::string & nodename = METADATA(nodename);
        std::string errmsg = "did not find class name after '" + nodename + "'.";
        PCERR << errmsg << std::endl;
        throw std::runtime_error( errmsg );
        return 0;
	}

{CLOSER} {
        PCERR << METADATA(internaldepth)<< " Closing node" << std::endl;
        BuilderContext::close_node( this );
        size_t & node_depth = METADATA(internaldepth);
        --node_depth;
        if( 0 == BuilderContext::node_depth( this ) )
        {
                // stop once we close the first top-level node.
                return 0;
        }
	}


^"(s11n::parens)" { continue;} // magic cookie

({SPACE})|\n {;}

. { PCERR << "Unexpected token: [" << YYText()<<"]"<<std::endl;}


%%
// ^{SPACE}*[#;].*$ {;} // comment lines
// [#;][^{OPENER}]* {;} // comments trailing after node closures



#undef METADATA
#undef PCERR
