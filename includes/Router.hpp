#ifndef ROUTER_HPP
# define ROUTER_HPP

# include <map>
# include <exception>
# include <sys/poll.h>
# include <Server.hpp>
# include <Socket.hpp>
# include <Request.hpp>

class Router {
	std::map<size_t, std::vector<Server *> >	_servers;
	std::map<size_t, std::vector<Server *> >	_clients;
	std::vector<pollfd>							_fds;
	std::vector<Socket>							_sockets;
	size_t										_serverFdsNumber;
	std::map<int, Request>						_requests;

	std::vector<Server*> getServers(size_t fd);
	std::vector<Server*> getServerWithClientFd(size_t clientFd);
	void	setServer(size_t fd, Server* server);
	void	setClient(size_t clientFd, size_t serverFd);
	void	removeClient(size_t fdIndex);
	void	managePollin(size_t fdIndex);
	void	managePollout(size_t fdIndex);
	void	closeSockets();
	bool	manageRequests(int fd, std::vector<Server *> servers, char const* buffer, ssize_t size);
	bool	executeRequest(int fd);
	void	deleteRequest(int fd);

public:
	~Router();
	void	initServerFds();
	void	initSockets(std::list<Server> &servers);
	int		pollFds();
	void	readEvents();

	class WrongFdException : public std::exception {
	public:
		const char* what() const throw();
	};
};

#endif
