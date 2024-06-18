#ifndef ROUTER_HPP
# define ROUTER_HPP

# include <map>
# include <exception>
# include <sys/poll.h>
# include <Socket.hpp>
# include <Server.hpp>

class Router {
	std::map<size_t, std::vector<Server *> > _servers;
	std::map<size_t, std::vector<Server *> >	_clients;
	std::vector<pollfd> 	_fds;
	std::vector<Socket>		_sockets;
	size_t					_serverFdsNumber;

	std::vector<Server*> getServers(size_t fd);
	std::vector<Server*> getServerWithClientFd(size_t clientFd);
	void	setServer(size_t fd, Server* server);
	void	setClient(size_t clientFd, size_t serverFd);
	void	removeClient(size_t fdIndex);
	int		managePollin(size_t fdIndex);
	int		managePollout(size_t fdIndex);
	void	closeSockets();
public:

	~Router();
	void	initServerFds();
	void	initSockets(std::list<Server> &servers);
	int		pollFds();
	int		readEvents();

	class WrongFdException : std::exception {
	public:
		const char* what() const throw();
	};
};


#endif