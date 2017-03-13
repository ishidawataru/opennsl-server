%option c++
%{

namespace { // to get doxygen to ignore these comments :/
    /**
       my first lex :)

       This code is for parsing Rusty Ballinger's "text mode
       serialization format" (or "funtxt", as we call it).

       Here's a rough sample of it's supported format:

       nodename class=ClassName
       {
           property1_name property1_value
           property2_name "value with spaces"
           propertyN_name 'or single quotes'
           foo this \
           	line is split \
                with backslashes. Literal backslashes must be \
                escaped, c-style, like this: \\
           # comment lines
           nodename class=SomeClass
           {
           ...
           }
       }

       Extensions to the original format, probably not tolerated by
       older code and should probably be left out:

       - comment lines can start with ; or # or //

       - C++-style comment blocks are supported.


     Known problems:

     - If node_depth() is non-zero when this code starts parsing then it
     does not work properly at all. In practice this has never happened.

    - add syntax extension:
        nodename class=SomeClass N
      where N is a number. That is, the node is replicated N times into the
      output.

    - add syntax extension:
       # nodename class=Foo
      will comment out the whole class block.


    */

} // namespace

#define YY_SKIP_YYWRAP 1
int yywrap() { return 1; }


#ifdef NDEBUG
#  undef NDEBUG // force assert()
#endif

#include <iostream>
#include <string>
#include <deque>
#include <cassert>
#include <stdio.h> // only for EOF definition


#include <s11n.net/s11n/io/strtool.hpp> // translate_entities()
#include <s11n.net/s11n/io/data_node_format.hpp> // node_tree_builder, parens_serializer, etc.
#include <s11n.net/s11n/io/funtxt_serializer.hpp> // funtext_sharing_context
#include <s11n.net/s11n/s11n_debuggering_macros.hpp> // CERR

#define LEXCERR if( 0 ) CERR << "funtxt lexer: "

#include <s11n.net/s11n/io/funtxt_data_nodeFlexLexer.hpp> // lexer base type

#define S11N_FACREG_TYPE funtxt_data_nodeFlexLexer
#define S11N_FACREG_TYPE_NAME "funtxt_data_nodeFlexLexer"
#define S11N_FACREG_INTERFACE_TYPE FlexLexer
#include <s11n.net/s11n/factory_reg.hpp>

using std::cin;
using std::cerr;
using std::cout;
using std::endl;




namespace  funtxt
{

        /**
           Runs key_value_parser::parse(str) and saves the result in the
           lexer's kvp object.  If this function returns false then
           kvp is in an undefined state, otherwise kvp holds a key and
           value parsed from str.

           Accepted formats:

           key value value value ...

           key = value value value ...

           key

           todo: return a std::pair instead of using key_value_parser
           (it's usage is legacy stuff).

        */
        bool parseKVP( const std::string & str, std::string & key, std::string & val )
        { // todo: clean this shit up!!!
                LEXCERR << "parseKVP(["<<str<<"])\n";
                std::string delim = " \t";

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
                ::s11n::io::strtool::translate_entities( val, s11n::io::funtxt_serializer_translations(), true );
                LEXCERR << "prop ["<<key<<"]=["<<val<<"]"<<std::endl;
                return true;
        }
}


namespace {
	typedef s11n::io::tree_builder_context<
                s11n::io::sharing::funtxt_sharing_context
        > BuilderContext;


} // anonymous namespace
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
%x IN_COMMENT_BLOCK

%%

"/*""*"* { // (* comment blocks *)
	BEGIN IN_COMMENT_BLOCK;
	/********************************************************************************
        // there is apparently an endless loop caused in some cases with this :/
        // c++-style comments. Code mostly taken from the flex info pages.
             int c;
             while((c = yyinput()) != 0)
             {
                     if(c == '*')
                     {
                             c = yyinput();
                             if( 0 == c || '/' == c ) break;
                     }
             }
             continue; //return 1;
	********************************************************************************/
        }


<IN_COMMENT_BLOCK>([^\\])?"*/" { /* this match isn't 100% correct.*/ BEGIN 0; }
<IN_COMMENT_BLOCK>[{SPACE}\n]+ { continue;}
<IN_COMMENT_BLOCK>. { continue;}
<IN_COMMENT_BLOCK>EOF {
        LEXCERR << "hit EOF in a (*comment block*)." << std::endl;
        return 0;
	}


({SPACE}*)([;#]|\/).* { continue; /* single-line comment */ }


({SPACE}*){WORD}{SPACE}+class=(({CLASSNAME})|({QUOTED_STRING})) { // e.g.: nodename class=foo::Bar
	std::string foo = YYText();
        ::s11n::io::strtool::trim_string( foo );
        LEXCERR << "class dec token=["<<foo<<"]"<<endl;
        std::string nodename = foo.substr( 0, foo.find_first_of( " \t" ) );
        std::string nodeclass = foo.substr( foo.find( "=" ) + 1 );
        assert( ! nodeclass.empty() );
        if( ('\"' == nodeclass[0]) || ('\'' == nodeclass[0]) )
        { // remove quotes from class name
                nodeclass.erase(0,1);
                nodeclass.resize(nodeclass.size()-1); // chop trailing quote
        }

        int ret = BuilderContext::open_node( this, nodeclass, nodename ) ? 1 : -1;
        if( (ret == -1) )
        {
                LEXCERR << "open_node("<<nodeclass<<","<<nodename<<") failed. Token="<<foo << std::endl;
                return 0;
        }
        continue; // return ret;
   }

\{ { // node's opening brace
        ++METADATA(internaldepth);
        // nothing.
        continue;
   }

\} { // node's closing brace
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
        const std::string & myproperty = METADATA(property);
	std::string key, val;
        if( ! funtxt::parseKVP( myproperty, key, val ) ) {
                LEXCERR << "failed parsing key/value pair from property token ["<<myproperty<<"]"<<endl;
        }
        BuilderContext::add_property( this, key, val );
        LEXCERR << "add_property(["<<key<<"],["<<val<<"])"<<std::endl;
        BEGIN INITIAL;
	}


{PROP_DECL_SPACE} { // property_name prop_val...
	if( METADATA(internaldepth) != BuilderContext::node_depth(this) ) { return 1; }
	METADATA(property) = YYText();
        BEGIN READ_PROPERTY_VALUE;
	}

.|\n|{SPACE}+ {;}

%%

#undef METADATA
