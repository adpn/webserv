#include <iostream> // cout
#include <vector>
#include <Config.hpp>
#include <Router.hpp>

int main(int argc, char **argv)
{
	std::string conf_path = "configuration_files/something1.conf";
	switch (argc){
		case 2:
			conf_path = argv[1];
		case 1:
			break;
		default:
			return 1;
	}

	Config config(conf_path);
	Router router;
	router.initServerFds(config.get_servers());

	std::cout << "Main loop start:" << std::endl;
	while (true)
	{
		if (router.pollFds() == -1) {
			std::cout << "Error poll";
			return 1;
		}

		if (router.readEvents())
			return 1;
	}
	return 0;
}
