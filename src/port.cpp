#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>

#include <s11n.net/shellish/shellish.hpp> // eshell framework
#include <s11n.net/shellish/argv_parser.hpp>
#include <s11n.net/shellish/arguments.hpp>
#include <s11n.net/shellish/builtins.hpp>

#ifndef __common_
#define __common_
#include "common.cpp"
#endif

extern "C" {
#include "opennsl/error.h"
#include "opennsl/port.h"
#include "opennsl/stat.h"
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
    ret = opennsl_stat_init(0);
    if(ret != OPENNSL_E_NONE) {
        shellish::ostream() <<  "opennsl_stat_init() failed " << opennsl_errmsg(ret);
        return 1;
    }
    return 0;
}

int PortStat(const shellish::arguments & args) {
    auto ret = opennsl_stat_sync(0);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_stat_sync() failed " << opennsl_errmsg(ret);
        return 1;
    }
    size_t i_len_args = args.argc();
    std::string tmp_port = args[i_len_args-1];
    int port = std::stoi(tmp_port);
    int nstat;
    opennsl_stat_val_t stat_arr[] = {
       opennsl_spl_snmpIfInUcastPkts,
       opennsl_spl_snmpIfInNUcastPkts,
       opennsl_spl_snmpIfInDiscards,
       opennsl_spl_snmpIfInErrors,
       opennsl_spl_snmpIfInMulticastPkts,
       opennsl_spl_snmpIfInBroadcastPkts,
       opennsl_spl_snmpIfOutUcastPkts,
       opennsl_spl_snmpIfOutNUcastPkts,
       opennsl_spl_snmpIfOutDiscards,
       opennsl_spl_snmpIfOutErrors,
       opennsl_spl_snmpIfOutMulticastPkts,
       opennsl_spl_snmpIfOutBroadcastPkts,};
    nstat = (sizeof(stat_arr) / sizeof(opennsl_stat_val_t));
    uint64 val[30] = {0};
    ret = opennsl_stat_multi_get(0, port, nstat, stat_arr, val);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        shellish::ostream() << "opennsl_stat_multi_get() failed " << opennsl_errmsg(ret);
        return 1;
    }
    shellish::ostream() << "Count of Ingress Unicast packets    .......   " << val[0] << std::endl;
    shellish::ostream() << "Count of Ingress Non-unicast packets.......   " << val[1] << std::endl;
    shellish::ostream() << "Count of Ingress Discard packets    .......   " << val[2] << std::endl;
    shellish::ostream() << "Count of Ingress Error packets      .......   " << val[3] << std::endl;
    shellish::ostream() << "Count of Ingress Multicast packets  .......   " << val[4] << std::endl;
    shellish::ostream() << "Count of Ingress Broadcast packets  .......   " << val[5] << std::endl;
    shellish::ostream() << "Count of Egress Unicast packets    .......   " << val[6] << std::endl;
    shellish::ostream() << "Count of Egress Non-unicast packets.......   " << val[7] << std::endl;
    shellish::ostream() << "Count of Egress Discard packets    .......   " << val[8] << std::endl;
    shellish::ostream() << "Count of Egress Error packets      .......   " << val[9] << std::endl;
    shellish::ostream() << "Count of Egress Multicast packets  .......   " << val[10] << std::endl;
    shellish::ostream() << "Count of Egress Broadcast packets  .......   " << val[11] << std::endl;
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
    std::unordered_map<std::string, shellish::command_handler_func> commands = { {"init", PortInit}, {"stat", PortStat}, {"no", PortShutdown}, {"shut", PortShutdown}, {"shutdown", PortShutdown}, {"exit", PortExit} };
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
