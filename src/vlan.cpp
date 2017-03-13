#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>

#include <s11n.net/shellish/shellish.hpp> // eshell framework
#include <s11n.net/shellish/argv_parser.hpp>
#include <s11n.net/shellish/arguments.hpp>
#include <s11n.net/shellish/builtins.hpp>

#include <s11n.net/shellish/shellish_debuggering_macros.hpp> // CERR


extern "C" {
#include "opennsl/error.h"
#include "opennsl/vlan.h"
}

bool MODE_VLAN = false;

int VLANCreate(const shellish::arguments & args) {
    size_t i_len_args = args.argc();
    std::string tmp_vid = args[i_len_args-1];
    int vid = std::stoi(tmp_vid);
    auto ret = opennsl_vlan_create(0, vid);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_port_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int VLANAddPort(const shellish::arguments & args) {
    size_t i_len_args = args.argc();
    std::string tmp_vid = args[i_len_args-1];
    int vid = std::stoi(tmp_vid);
    std::string tmp_port = args[i_len_args-2];
    int port = std::stoi(tmp_port);
    auto ret = opennsl_vlan_gport_add(0, vid, port, 0);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_port_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int VLANDelPort(const shellish::arguments & args) {
    size_t i_len_args = args.argc();
    std::string tmp_vid = args[i_len_args-1];
    int vid = std::stoi(tmp_vid);
    std::string tmp_port = args[i_len_args-2];
    int port = std::stoi(tmp_port);
    auto ret = opennsl_vlan_gport_delete(0, vid, port);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_port_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int VLANDestroy(const shellish::arguments & args) {
    size_t i_len_args = args.argc();
    std::string tmp_vid = args[i_len_args-1];
    int vid = std::stoi(tmp_vid);
    auto ret = opennsl_vlan_destroy(0, vid);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_port_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int VLANExit (const shellish::arguments & args) { 
    MODE_VLAN = false;
    shellish::ostream() << "VLAN Mode: Deactivated" << std::endl;
    return 0; 
}

int VLAN(const shellish::arguments & args) {
    std::unordered_map<std::string, shellish::command_handler_func> commands = { {"create", VLANCreate}, {"add", VLANAddPort}, {"delete", VLANDelPort}, {"destroy", VLANDestroy}, {"exit", VLANExit} };
    if (MODE_VLAN) {
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
            MODE_VLAN = true;
            shellish::ostream() << "VLAN Mode: Activated" << std::endl;
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