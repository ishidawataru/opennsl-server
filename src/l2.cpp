#include <string>
#include <iostream>
#include <cstring>
#include <unordered_map>


#include <s11n.net/shellish/shellish.hpp> // eshell framework
#include <s11n.net/shellish/argv_parser.hpp>
#include <s11n.net/shellish/arguments.hpp>
#include <s11n.net/shellish/builtins.hpp>

#include <s11n.net/shellish/shellish_debuggering_macros.hpp> // CERR

#include "common.cpp"

extern "C" {
#include "opennsl/error.h"
#include "opennsl/l2.h"
}


bool MODE_L2 = false;

opennsl_l2_addr_t get_l2_addr(const shellish::arguments & args) {
    opennsl_l2_addr_t ret;
    opennsl_mac_t mac;
    int vid;
    int port;
    std::string tmp_mac;
    std::string tmp_vid = "1";
    std::string tmp_port;
    size_t i_len_args = args.argc();
    if ((i_len_args == 4) && (args[0] == "add")) {
        tmp_mac = (args[i_len_args-3]);
        tmp_vid = (args[i_len_args-1]);
        tmp_port = (args[i_len_args-2]);
    } else if ((i_len_args == 4) && (args[0] == "l2")) {
        tmp_mac = (args[i_len_args-2]);
        tmp_port = (args[i_len_args-1]);
    } else if ((i_len_args == 5) && (args[0] == "l2")) {
        tmp_mac = (args[i_len_args-3]);
        tmp_vid = (args[i_len_args-1]);
        tmp_port = (args[i_len_args-2]);
    } else {
        tmp_port = (args[i_len_args-1]);
        tmp_mac =  (args[i_len_args-2]);
    }
    unsigned char* baseMac = mac_convert_str_to_bytes(tmp_mac);
    port = std::stoi(tmp_port);
    vid = std::stoi(tmp_vid);
    memcpy(mac, baseMac, 6);
    opennsl_l2_addr_t_init(&ret, mac, vid);
    ret.flags = (OPENNSL_L2_L3LOOKUP | OPENNSL_L2_STATIC);
    ret.port = port;
    return ret;
}

int L2Init(const shellish::arguments & args) {
    auto ret = opennsl_l2_cache_init(0);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_l2_cache_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int L2AddAddress (const shellish::arguments & args) { 
    auto addr = get_l2_addr(args);
    auto ret = opennsl_l2_addr_add(0, &addr);
    if ( ret != OPENNSL_E_NONE ) {
        shellish::ostream() << "opennsl_l2_addr_add() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int L2DeleteAddress (const shellish::arguments & args) {
    opennsl_mac_t mac;
    std::string tmp_mac;
    std::string tmp_vid = "1";
    size_t i_len_args = args.argc();
    if (i_len_args > 2) {
        tmp_mac = (args[i_len_args-2]);
        tmp_vid = (args[i_len_args-1]);
    } else {
        tmp_mac = (args[i_len_args-1]);
    }
    unsigned char* baseMac = mac_convert_str_to_bytes(tmp_mac);
    std::memcpy(mac, baseMac, 6);
    int vid = std::stoi(tmp_vid);
    auto ret = opennsl_l2_addr_delete(0, mac, vid);
    if ( ret != OPENNSL_E_NONE ) {
        shellish::ostream() << "opennsl_l2_addr_delete() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int trav_fn(int unit, opennsl_l2_addr_t *info, void *user_data) {
    std::string mac;
    mac = mac_convert_bytes_to_sting(info->mac);
    shellish::ostream() << mac << " - " << info->port << " - " << info->vid <<std::endl;
    return 0;
}

int L2ListAddress (const shellish::arguments & args) { 
    void *n;
    auto ret = opennsl_l2_traverse(0, trav_fn, n);
    if ( ret != OPENNSL_E_NONE ) {
        shellish::ostream() << "opennsl_l2_traverse() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int L2Exit (const shellish::arguments & args) { 
    MODE_L2 = false;
    shellish::ostream() << "L2 Mode: Deactivated" << std::endl;
    return 0; 
}

int L2(const shellish::arguments & args) {
    std::unordered_map<std::string, shellish::command_handler_func> commands = { {"init", L2Init}, {"add", L2AddAddress}, {"delete", L2DeleteAddress}, {"list", L2ListAddress}, {"exit", L2Exit} };
    if (MODE_L2) {
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
            MODE_L2 = true;
            shellish::ostream() << "L2 Mode: Activated" << std::endl;
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
