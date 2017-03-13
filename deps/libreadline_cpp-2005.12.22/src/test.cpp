
#include "Readline.hpp"
#include <iostream>
#define CERR std::cerr << __FILE__<<":"<<__LINE__<<": "
int main( int argc, char ** argv )
{

        using namespace readlinecpp;
        Readline rl;

        bool breakout = false;
        std::string line;
        CERR << "Readline test. Underlying readline implementation: " << Readline::rl_implementation_name() << "\n";
        while( ! breakout )
        {
                line = rl.readline( "test prompt : ", breakout );
                if( breakout ) break;
                CERR << "got: [" << line << "]\n";
        }
        std::cerr << "Bye!\n";

        return 0;
}
