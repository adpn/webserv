#include <Router.hpp>
#include <Request.hpp>
#include "Socket.hpp"

#define BUFFER_SIZE 1024

Router::~Router() {
	closeSockets();
}

/* Exceptions*/
const char* Router::WrongFdException::what() const throw() {
	return "wrong fd received";
}
/* Getters and setters */

// REAL FD AS INPUT NOT FDS INDEX
std::vector<Server*> Router::getServers(size_t fd) {
	if (fd < 3)
		throw WrongFdException();
	std::map<size_t, std::vector<Server *> >::iterator it = _servers.find(fd);
	if (it == _servers.end())
		throw WrongFdException();
	return (*it).second;
}

// REAL FD AS INPUT NOT FDS INDEX
std::vector<Server*> Router::getServerWithClientFd(size_t clientFd) {
	if (clientFd < 3)
		throw WrongFdException();
	std::map<size_t, std::vector<Server*> >::iterator it = _clients.find(clientFd);
	if (it == _clients.end())
		throw WrongFdException();
	return (*it).second;
}

// REAL FD AS INPUT NOT FDS INDEX
void	Router::setClient(size_t clientFd, size_t serverFd) {
	if (clientFd < 3)
		throw WrongFdException();
	_clients.insert(std::pair<size_t, std::vector<Server*> >(clientFd, getServers(serverFd)));
}

void	Router::removeClient(size_t fdIndex) {
	std::map<size_t, std::vector<Server*> >::iterator it = _clients.find(_fds[fdIndex].fd);
	if (it != _clients.end()) {
    	_clients.erase(it);
	}
	close(_fds[fdIndex].fd);
	_fds.erase(_fds.begin() + fdIndex);
}
/* Method functions */

int Router::managePollin(size_t fdIndex)
{
	// case where the POLLIN is on a server fd
	if (fdIndex < _serverFdsNumber) {
		std::cout << "POLLIN SERVER" << std::endl;
		// Accept new client connection
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		int clientFd = accept(_fds[fdIndex].fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
		if (clientFd == -1) {
			std::cout << "Accept error" << std::endl;
			return 0;
		}
		std::cout << "Client connected to fd : " << _fds[fdIndex].fd << std::endl;

		// Add new client socket to fds
		struct pollfd pfd;
		pfd.fd = clientFd;
		pfd.events = POLLIN; // Initially only enable POLLIN
		setClient(clientFd, _fds[fdIndex].fd);
		_fds.push_back(pfd);

	}
	else {
		std::cout << "POLLIN CLIENT: " << _fds[fdIndex].fd << std::endl;
		// case where the POLLIN is on a client fd
		char buffer[BUFFER_SIZE];
		ssize_t bytesReceived = recv(_fds[fdIndex].fd, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			// std::cout << "Received from client: " << std::endl;
			// std::cout << buffer << std::endl;
			Request::manageRequests(_fds[fdIndex].fd, getServerWithClientFd(_fds[fdIndex].fd), buffer, bytesReceived);

			// manage client
			_fds[fdIndex].events |= POLLOUT; // Enable POLLOUT to send data
		}
		else if (bytesReceived == 0) {
			// Connection closed by client
			std::cout << "Client disconnected " << _fds[fdIndex].fd <<std::endl;

			removeClient(fdIndex);
			--fdIndex;
		}
		else {
			std::cout << "Recv error" << std::endl;
			return 1;
		}
	}
	return 0;
}

int Router::managePollout(size_t fdIndex) {
	std::cout << "Ready to send data to client on fd " << _fds[fdIndex].fd << std::endl;
	// Send data to client
	Request::executeRequest(_fds[fdIndex].fd);
	std::cout << "Client disconnected after sending a response: " << _fds[fdIndex].fd <<std::endl;
	_fds[fdIndex].events = POLLIN;

	removeClient(fdIndex);
	--fdIndex;
	return 0;
}

int Router::pollFds() {
	// Last argument is timeout time
	return poll(&_fds[0], _fds.size(), 10000);
}

int	Router::readEvents() {
	/*
	fds = array of all fd
	go through all the fds to catch received events, especially on servers fds to detect new connection
	*/
	for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].revents & POLLIN) {
			if (managePollin(i))
				return 1;
		}
		else if (_fds[i].revents & POLLOUT) {
			if (managePollout(i))
					return 1;
		}
	}
	return 0;
}

// Init poll fds
void Router::initServerFds() {
	size_t i = 0;

	std::cout << "Init poll fds:" << std::endl;
	for (i = 0; i < _sockets.size(); i++) {
		std::cout << "Adding socket " << _sockets[i] << " in fds" << std::endl;
		struct pollfd pfd;
		pfd.fd = _sockets[i]; // first elements are for servers sockets
		pfd.events = POLLIN; // server sockets is just to check events, real communication is done with clients sockets
		_fds.push_back(pfd);
	}
	_serverFdsNumber = i;

	std::cout << "Total server fds : " << _serverFdsNumber << std::endl;
}

void	Router::initSockets(std::list<Server> &servers) {
	std::cout << "Init sockets:" << std::endl;
	std::map<unsigned int, std::vector<Server*> > portsMap;
	//Going through all servers
	for (std::list<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
		std::vector<unsigned int> ports = it->get_port();
		
		//Adding server reference in the map
		for (size_t j = 0; j < ports.size(); j++) {
			portsMap[ports[j]].push_back(&(*it));
		}
	}

	//Creating all the sockets
	for (std::map<unsigned int, std::vector<Server*> >::iterator it = portsMap.begin(); it != portsMap.end(); it++) {
		Socket socket(it->first);
		_sockets.push_back(socket);

		_servers[socket.getFd()] = it->second;
	}

	std::cout << "End if init sockets" << std::endl << std::endl;
}

void	Router::closeSockets() {
	for (size_t i = 0; i < _sockets.size(); i++) {
		std::cout << "Closing port: " << _sockets[i].getPort() << " with socket fd: " << _sockets[i] << std::endl;
		close(_sockets[i]);
	}
}
