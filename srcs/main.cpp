#include <iostream> // cout
#include <unistd.h>
#include <vector>
#include <csignal>
#include <Config.hpp>
#include <Router.hpp>
#include <exception>

bool	checkSig(bool check = false) {
	static bool SigCalled = false;

	if (SigCalled == true)
		return true;
	SigCalled = check;
	return SigCalled;
}

void	handleSig(int sig) {
	(void) sig;
	checkSig(true);
}

int main(int argc, char **argv)
{
	std::string conf_path = "configuration_files/2servers.conf";
	switch (argc){
		case 2:
			conf_path = argv[1];
		case 1:
			break;
		default:
			return 1;
	}
	try {
		Config config(conf_path);
		Router router;
		router.initSockets(config.get_servers());
		router.initServerFds();
		std::signal(SIGINT, handleSig);

		if (chdir("website")) {
			std::cout << "Error: can't access website files" << std::endl;
			return 1;
		}

		std::cout << "Main loop start:" << std::endl;
		while (true)
		{

			if (router.pollFds() == -1) {
				if (checkSig())
					return 0;
				std::cout << "Error: poll fail" << std::endl;
				return 1;
			}

			router.readEvents();
		}
	}
	catch (std::exception const& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
