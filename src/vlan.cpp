#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>

#include <s11n.net/shellish/shellish.hpp> // eshell framework
#include <s11n.net/shellish/argv_parser.hpp>
#include <s11n.net/shellish/arguments.hpp>
#include <s11n.net/shellish/builtins.hpp>

#include <s11n.net/shellish/shellish_debuggering_macros.hpp> // CERR
#include "common.cpp"


extern "C" {
#include "opennsl/error.h"
#include "opennsl/vlan.h"
#include "opennsl/types.h"
}

bool MODE_VLAN = false;


int VLANCreate(const shellish::arguments & args) {
    size_t i_len_args = args.argc();
    std::string tmp_vid = args[i_len_args-1];
    int vid = std::stoi(tmp_vid);
    auto ret = opennsl_vlan_create(0, vid);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_vlan_create() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int VLANAddPort(const shellish::arguments & args) {
    size_t i_len_args = args.argc();
    size_t offset;
    bool untag = true;
    if (args[i_len_args-1] != "tagged"){
        offset = i_len_args;
    } else {
        untag = false;
        offset = i_len_args-1;
    }
    std::string tmp_vid = args[offset-1];
    int vid = std::stoi(tmp_vid);
    opennsl_pbmp_t pbmp;
    opennsl_pbmp_t upbmp;
    std::string tmp_port = args[offset-2];
    int port = std::stoi(tmp_port);
    OPENNSL_PBMP_CLEAR(pbmp);
    OPENNSL_PBMP_CLEAR(upbmp);
    OPENNSL_PBMP_PORT_ADD(pbmp, port);
    if (untag) {
        OPENNSL_PBMP_PORT_ADD(upbmp, port);
    }
    auto ret = opennsl_vlan_port_add(0, vid, pbmp, upbmp);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_vlan_port_add() failed " << opennsl_errmsg(ret);
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
    opennsl_pbmp_t pbmp;
    OPENNSL_PBMP_CLEAR(pbmp);
    OPENNSL_PBMP_PORT_ADD(pbmp, port);
    auto ret = opennsl_vlan_port_remove(0, vid, pbmp);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_vlan_port_remove() failed " << opennsl_errmsg(ret);
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
        shellish::ostream() << "opennsl_vlan_destroy() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int VLANList(const shellish::arguments & args) {
    opennsl_vlan_data_t *listp; 
    int count;
    auto ret = opennsl_vlan_list(0, &listp, &count);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_vlan_list() failed " << opennsl_errmsg(ret);
        return 1;
    }
    for (auto i = 0 ; i < count; i++) {
        shellish::ostream() << "VID:" <<listp[i].vlan_tag << std::endl;
        shellish::ostream() << "Members:";
        int port;
        OPENNSL_PBMP_ITER( listp[i].port_bitmap, port) {
            shellish::ostream() << port << ", " ;
        }
        shellish::ostream() << std::endl;
        shellish::ostream() << "Untagged:";
        port = 0;
        OPENNSL_PBMP_ITER( listp[i].ut_port_bitmap, port) {
            shellish::ostream() << port << ", ";
        }
        shellish::ostream() << std::endl << std::endl;
    }
    ret = opennsl_vlan_list_destroy(0, listp, count);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_vlan_list_destroy() failed " << opennsl_errmsg(ret);
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
    std::unordered_map<std::string, shellish::command_handler_func> commands = { {"create", VLANCreate}, {"add", VLANAddPort}, {"delete", VLANDelPort}, {"destroy", VLANDestroy}, {"list", VLANList}, {"exit", VLANExit} };
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