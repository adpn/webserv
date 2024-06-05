#include "Server.hpp"
#include "Config.hpp"

int main(void){
	
	try {
		Config	configuration("something1.conf");
		configuration.bufferize();
		std::vector<Server> servers = configuration.get_servers();
		if (servers.empty())
			std::cout << "No server found" << std::endl;
		for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++){
			std::cout << *it;
		}
	}
	catch ( std::exception & e ){
		std::cout << e.what() << std::endl;
	}

	return 0;
}