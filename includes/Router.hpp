#ifndef ROUTER_HPP
# define ROUTER_HPP

# include <Server.hpp>
# include <map>
# include <exception>
# include <sys/poll.h>

class Router {
	std::map<size_t, Server &> _servers;
	std::map<size_t, Server &>	_clients;
	std::vector<pollfd> 	_fds;
	size_t					_serverFdsNumber;

	Server& getServer(size_t fd);
	Server& getServerWithClientFd(size_t clientFd);
	void	setServer(size_t fd, Server& server);
	void	setClient(size_t clientFd, size_t serverFd);
	void	removeClient(size_t fdIndex);
	int		managePollin(size_t fdIndex);
	int		managePollout(size_t fdIndex);
public:
	
	void	initServerFds(std::vector<Server>& servers);
	int		pollFds();
	int		readEvents();

	class WrongFdException : std::exception {
	public:
		const char* what() const throw();
	};
};


#endif