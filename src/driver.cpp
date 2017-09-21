#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>

#include <s11n.net/shellish/shellish.hpp> // eshell framework
#include <s11n.net/shellish/argv_parser.hpp>
#include <s11n.net/shellish/arguments.hpp>
#include <s11n.net/shellish/builtins.hpp>

extern "C" {
#include "opennsl/error.h"
#include "sal/driver.h"
}

bool MODE_DRIVER = false;

int DriverInit(const shellish::arguments & args) {
    opennsl_init_t conf = opennsl_init_t();
    auto ret = opennsl_driver_init(&conf);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_driver_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    if (!opennsl_rx_active(0)) {
        shellish::ostream() << "starting opennsl rx layer" << std::endl;
        auto rv = opennsl_rx_start(0, NULL);
        if (OPENNSL_FAILURE(rv)){
            std::ostringstream err;
            shellish::ostream() << "opennsl_rx_start() failed " << opennsl_errmsg(rv);
            return 2;
        }
    }
    return 0;
}

int DriverExit (const shellish::arguments & args) { 
    MODE_DRIVER = false;
    shellish::ostream() << "Driver Mode: Deactivated" << std::endl;
    return 0; 
}

int DRIVER(const shellish::arguments & args) {
    std::unordered_map<std::string, shellish::command_handler_func> commands = { {"init", DriverInit}, {"exit", DriverExit} };
    if (MODE_DRIVER) {
        auto search = commands.find(args[0]);
        if (search != commands.end()) {
            return search->second(args);
        } else {
            shellish::ostream() << "Command Error: \n User inputted: ";
            for( size_t i = 0; i < args.argc(); i++ ) {
                shellish::ostream() <<args[i] << " ";
            }
            shellish::ostream() << std::endl << std::endl << "Accepted commands are: " << std::endl;
            for (auto i = commands.begin(); i != commands.end(); ++i) {
                auto cur = i->first;
                shellish::ostream() << cur << std::endl;
            }
            return 0;
            }
    } else {
        if (args.argc() == 1){
            MODE_DRIVER = true;
            shellish::ostream() << "Driver Mode: Activated" << std::endl;
            return 0;
        } else {
            auto search = commands.find(args[1]);
            if (search != commands.end()) {
                return search->second(args);
            } else {
            shellish::ostream() << std::endl << std::endl << "Accepted commands are: " << std::endl;
            for (auto i = commands.begin(); i != commands.end(); ++i) {
                auto cur = i->first;
                shellish::ostream() << commands[cur] << std::endl;
            }
            return 0;
            }
        }
    }
}