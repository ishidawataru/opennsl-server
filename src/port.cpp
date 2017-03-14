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
#include "opennsl/port.h"
#include "examples/util.h"
}

bool MODE_PORT = false;

int PortInit(const shellish::arguments & args) {
    auto ret = opennsl_port_init(0);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_port_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    ret = example_port_default_config(0);
    if (ret != OPENNSL_E_NONE) {
        shellish::ostream() <<  "example_port_default_config() failed " << opennsl_errmsg(ret);
        return 1;
    }
    ret = example_switch_default_vlan_config(0);
    if(ret != OPENNSL_E_NONE) {
        shellish::ostream() <<  "example_port_default_vlan_config() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int PortShutdown(const shellish::arguments & args) {
    size_t i_len_args = args.argc();
    std::string tmp_shut = args[0];
    int shutdown = (int) (tmp_shut == "no");
    std::string tmp_port = args[i_len_args-1];
    int port = std::stoi(tmp_port);
    auto ret = opennsl_port_enable_set (0, port, shutdown);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_port_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int PortExit (const shellish::arguments & args) { 
    MODE_PORT = false;
    shellish::ostream() << "Port Mode: Deactivated" << std::endl;
    return 0; 
}

int PORT(const shellish::arguments & args) {
    std::unordered_map<std::string, shellish::command_handler_func> commands = { {"init", PortInit}, {"no", PortShutdown}, {"shut", PortShutdown}, {"shutdown", PortShutdown}, {"exit", PortExit} };
    if (MODE_PORT) {
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
            MODE_PORT = true;
            shellish::ostream() << "Port Mode: Activated" << std::endl;
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