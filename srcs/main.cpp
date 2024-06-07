#include <iostream> // cout
#include <vector>
#include <Config.hpp>
#include <Router.hpp>

int main(int argc, char **argv)
{
	if (argc > 2)
		return 1;

	std::string conf_path;
	if (argc == 1)
		conf_path = "configuration_files/something.conf";
	else
		conf_path = argv[1];

	Config config(conf_path);
	config.bufferize();

	std::vector<Server> servers;
	servers = config.get_servers();

	Router router;
	router.initServerFds(servers);

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
