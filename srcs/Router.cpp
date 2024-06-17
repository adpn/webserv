#include <Router.hpp>
#include <Request.hpp>
#include "Socket.hpp"

//buffer size should be defined with client_max_body_size ????????
#define BUFFER_SIZE 1024

/* Exceptions*/
const char* Router::WrongFdException::what() const throw() {
	return "wrong fd received";
}
/* Getters and setters */

// REAL FD AS INPUT NOT FDS INDEX
Server& Router::getServer(size_t fd) {
	if (fd < 3)
		throw WrongFdException();
	std::map<size_t, Server&>::iterator it = _servers.find(fd);
	if (it == _servers.end())
		throw WrongFdException();
	return (*it).second;
}

// REAL FD AS INPUT NOT FDS INDEX
Server& Router::getServerWithClientFd(size_t clientFd) {
	if (clientFd < 3)
		throw WrongFdException();
	std::map<size_t, Server&>::iterator it = _clients.find(clientFd);
	if (it == _clients.end())
		throw WrongFdException();
	return (*it).second;
}

// REAL FD AS INPUT NOT FDS INDEX
void	Router::setServer(size_t fd, Server& server) {
	_servers.insert(std::pair<size_t, Server&>(fd, server));
}

// REAL FD AS INPUT NOT FDS INDEX
void	Router::setClient(size_t clientFd, size_t serverFd) {
	if (clientFd < 3)
		throw WrongFdException();
	_clients.insert(std::pair<size_t, Server&>(clientFd, getServer(serverFd)));
}

void	Router::removeClient(size_t fdIndex) {
	std::map<size_t, Server&>::iterator it = _clients.find(_fds[fdIndex].fd);
	if (it != _clients.end()) {
    	_clients.erase(it);
	}
	close(_fds[fdIndex].fd);
	_fds.erase(_fds.begin() + fdIndex);
}

void Router::initServerFds(std::list<Server>& servers) {
	int pfdIndex = 0;
	std::cout << "Server for loop start:" << std::endl;
	for (std::list<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
		std::vector<Socket> serverSockets;

		serverSockets = it->get_sockets();
		std::cout << "Loop for server : " << it->get_name()[0] << std::endl;
		for (size_t j = 0; j < serverSockets.size(); j++)
		{
			std::cout << "Adding socket " << serverSockets[j] << " in fds" << std::endl;
			struct pollfd pfd;
			pfd.fd = serverSockets[j]; // first elements are for servers sockets
			pfd.events = POLLIN; // server sockets is just to check events, real communication is done with clients sockets
			_fds.push_back(pfd);
			setServer(serverSockets[j], *it);
			pfdIndex++;
		}
	}
	_serverFdsNumber = pfdIndex;
	std::cout << "Total server fds : " << _serverFdsNumber << std::endl;
}

/* Method functions */

int Router::managePollin(size_t fdIndex)
{
	std::vector<Socket> serverSockets;

	if (fdIndex < _serverFdsNumber)
		serverSockets = getServer(_fds[fdIndex].fd).get_sockets();
	else
		serverSockets = getServerWithClientFd(_fds[fdIndex].fd).get_sockets();

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
