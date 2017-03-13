#include <s11n.net/shellish/shellish.hpp> // eshell framework
#include <s11n.net/shellish/argv_parser.hpp>
#include <s11n.net/shellish/arguments.hpp>
#include <s11n.net/shellish/builtins.hpp>


#include <string>
#include <functional>
#include "l2.cpp"
#include "knet.cpp"
#include "port.cpp"
#include "vlan.cpp"
#include "driver.cpp"

//globals
bool MODE_C = false;
std::string prompt = "osh > "; 

// client-written command handler:

int command_list( const shellish::arguments & args) {
	std::string tasks = "exit \n";
	if (MODE_C &! (MODE_L2 || MODE_KNET || MODE_VLAN || MODE_DRIVER)) {
		tasks += "knet \n";
		tasks += "vlan \n";
		tasks += "port \n";
		tasks += "l2 \n";
		tasks += "driver \n";
	} else if (MODE_L2) {
		return L2(args);
	} else if (MODE_KNET) {
		return KNET(args);
	} else if (MODE_PORT) {
		return PORT(args);
	} else if (MODE_VLAN) {
		return VLAN(args);
	} else if (MODE_DRIVER) {
		return DRIVER(args);
	}else {
		tasks += "configure terminal\n";
		tasks += "show \n";
	}
	shellish::ostream() << tasks <<std::endl;
	return 0;
}



int default_handler(const shellish::arguments & args) {
	if (MODE_L2){
		return L2(args);
	} else if (MODE_KNET){
		return KNET(args);
	} else if (MODE_PORT){
		return PORT(args);
	} else if (MODE_VLAN){
		return VLAN(args);
	} else if (MODE_DRIVER){
		return DRIVER(args);
	} else {
		shellish::ostream() << "Command Error: \n User inputted: ";
		for( size_t i = 0; i < args.argc(); i++ ) {
			shellish::ostream() <<args[i] << " ";
		}
		shellish::ostream() << std::endl << std::endl << "Accepted commands are: " << std::endl;
		return command_list(args);
	}
}



int configure_wrapper(const shellish::command_handler_func handler, const shellish::arguments & args) {
	if (MODE_C){
		return handler(args);
	} else {
		shellish::ostream() << "Command Error: \n User inputted: ";
		for( size_t i = 0; i < args.argc(); i++ ) {
			shellish::ostream() <<args[i] << " ";
		}
		shellish::ostream() << std::endl << std::endl << "Accepted commands are: " << std::endl;
		return command_list(args);
	}
}

auto l2_handler = [](const shellish::arguments & args){
	if ((args.argc() == 1) && (args[0] == "l2")) {
		prompt = "osh configure l2> ";
	}
	return configure_wrapper(L2, args);
};

auto knet_handler = [](const shellish::arguments & args){
	if ((args.argc() == 1) && (args[0] == "knet")) {
		prompt = "osh configure knet> ";
	}
	return configure_wrapper(KNET, args);
};

auto port_handler = [](const shellish::arguments & args){
	if ((args.argc() == 1) && (args[0] == "port")) {
		prompt = "osh configure port> ";
	}
	return configure_wrapper(PORT, args);
};

auto vlan_handler = [](const shellish::arguments & args){
	if ((args.argc() == 1) && (args[0] == "vlan")) {
		prompt = "osh configure vlan> ";
	}
	return configure_wrapper(VLAN, args);
};

auto driver_handler = [](const shellish::arguments & args){
	if ((args.argc() == 1) && (args[0] == "driver")) {
		prompt = "osh configure driver> ";
	}
	return configure_wrapper(DRIVER, args);
};

int configure_handler(const shellish::arguments & args) {
	if (((args[0] == "conf") && (args[1] == "t")) || ((args[0] == "configure") && (args[1] == "terminal"))){
		MODE_C = true;
		shellish::ostream() << "Configure Mode: Activated" << std::endl;
		prompt = "osh configure > ";
	} else {
		shellish::ostream() << "Command Error: \n User inputted: ";
		for( size_t i = 0; i < args.argc(); i++ ) {
        	shellish::ostream() <<args[i] << " ";
    	}
    	shellish::ostream() << std::endl << std::endl << "Perhaps you meant: ";
		if (args[0] == "conf") {
			shellish::ostream() << "conf t" << std::endl;
		} else {
			shellish::ostream() << "configure terminal" << std::endl;
		}
	}
	return 0;
}

int exit_handler(const shellish::arguments & args) {
	if (MODE_C &! (MODE_L2 || MODE_KNET || MODE_PORT || MODE_DRIVER || MODE_VLAN)) {
		MODE_C = false;
		shellish::ostream() << "Configure Mode: Deactivated" << std::endl;
		prompt = "osh > ";
	} else if (MODE_L2) {
		prompt = "osh configure > ";
		return L2Exit(args);
	} else if (MODE_PORT) {
		prompt = "osh configure > ";
		return PortExit(args);
	} else if (MODE_KNET) {
		prompt = "osh configure > ";
		return KNETExit(args);
	} else if (MODE_VLAN) {
		prompt = "osh configure > ";
		return VLANExit(args);
	} else if (MODE_DRIVER) {
		prompt = "osh configure > ";
		return DriverExit(args);
	} else {
		shellish::handle_quit(args);
	}
	return 0;
}


int main( int argc, char ** argv ) {
    shellish::argv_parser & args = shellish::init( argc, argv );
    shellish::map_commander( "*", default_handler);
    shellish::map_commander( "help", command_list);
	shellish::map_commander( "?", command_list);
    shellish::map_commander( "exit", exit_handler);
	shellish::map_commander( "l2", l2_handler);
	shellish::map_commander( "port", port_handler);
	shellish::map_commander( "knet", knet_handler);
	shellish::map_commander( "vlan", vlan_handler);
	shellish::map_commander( "driver", driver_handler);
    shellish::map_commander( "configure", configure_handler);
    shellish::map_commander( "conf",  configure_handler);
    shellish::input_loop( prompt );
    return 0;
}
