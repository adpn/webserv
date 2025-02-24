#include <iostream>
#include <unistd.h>
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
	std::cout << std::endl;
	checkSig(true);
}

int main(int argc, char **argv)
{
	std::string conf_path = "configuration_files/2servers.conf";
	switch (argc) {
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
			throw std::runtime_error("can't access website files");
		}
		while (true)
		{

			if (router.pollFds() == -1) {
				throw std::runtime_error("poll fail");
			}

			router.readEvents();
		}
	}
	catch (std::exception const& e) {
		if (checkSig())
			exit(0);
		std::cout << "Error: " << e.what() << std::endl;
		exit(1);
	}

	exit(0);
	return 0;
}
