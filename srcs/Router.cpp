#include <Router.hpp>

#define BUFFER_SIZE 1024

Router::~Router() {
	std::cout << std::endl;
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

void Router::managePollin(size_t fdIndex)
{
	// case where the POLLIN is on a server fd
	if (fdIndex < _serverFdsNumber) {
		// Accept new client connection
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		int clientFd = accept(_fds[fdIndex].fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
		if (clientFd == -1) {
			std::cout << "Accept error" << std::endl;
			return ;
		}
		// Add new client socket to fds
		struct pollfd pfd;
		pfd.fd = clientFd;
		pfd.events = POLLIN;
		setClient(clientFd, _fds[fdIndex].fd);
		_fds.push_back(pfd);

	}
	else {
		// case where the POLLIN is on a client fd
		char buffer[BUFFER_SIZE];
		ssize_t bytesReceived = recv(_fds[fdIndex].fd, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			// std::cout << "Received from client: " << std::endl;
			// std::cout << buffer << std::endl;
			manageRequests(_fds[fdIndex].fd, getServerWithClientFd(_fds[fdIndex].fd), buffer, bytesReceived);

			// manage client
			_fds[fdIndex].events |= POLLOUT; // Enable POLLOUT to send data
		}
		else {
			// Connection closed by client
			//std::cout << "Client disconnected " << _fds[fdIndex].fd << std::endl;

			removeClient(fdIndex);
			deleteRequest(_fds[fdIndex].fd);
			--fdIndex;
		}
	}
}

void Router::managePollout(size_t fdIndex) {
	// Send data to client
	try {
		executeRequest(_fds[fdIndex].fd);
		removeClient(fdIndex);
		--fdIndex;
	}
	catch (std::exception const &e) {
		std::cout << "Client disconnected after send failed: " << _fds[fdIndex].fd << std::endl;
		removeClient(fdIndex);
		deleteRequest(_fds[fdIndex].fd);
		--fdIndex;
	}
}

int Router::pollFds() {
	// Last argument is timeout time
	return poll(&_fds[0], _fds.size(), 6000);
}

void 	Router::readEvents() {
	/*
	fds = array of all fd
	go through all the fds to catch received events, especially on servers fds to detect new connection
	*/
	for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].revents & POLLIN) {
			managePollin(i);
		}
		else if (_fds[i].revents & POLLOUT) {
			managePollout(i);
		}
	}
}

// Init poll fds
void Router::initServerFds() {
	size_t i = 0;

	for (i = 0; i < _sockets.size(); i++) {
		struct pollfd pfd;
		pfd.fd = _sockets[i];
		pfd.events = POLLIN;
		_fds.push_back(pfd);
	}
	_serverFdsNumber = i;

	std::cout << "Total server fds : " << _serverFdsNumber << std::endl;
}

void	Router::initSockets(std::list<Server> &servers) {
	std::map<unsigned int, std::vector<Server*> > portsMap;
	//Going through all servers
	for (std::list<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
		std::vector<unsigned int> ports = it->get_port();

		//Adding server pointers in the map
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
}

void	Router::closeSockets() {
	for (size_t i = 0; i < _sockets.size(); i++) {
		std::cout << "Closing port: " << _sockets[i].getPort() << " with socket fd: " << _sockets[i] << std::endl;
		close(_sockets[i].getFd());
	}
}

// returns true if request is finished
// return false if request is not finished or fd is bad
bool Router::manageRequests(int fd, std::vector<Server *> servers, char const* buffer, ssize_t size)
{
	if (fd < 3)
		return false;
	std::string package(buffer, buffer + size);
	std::map<int, Request>::iterator it = (_requests.insert(std::pair<int, Request>(fd, Request(fd, servers)))).first;
	Request& instance = (*it).second;
	instance.parse(package);
// std::cout << ">> parsed a packet[" << fd << "], " << instance._content_left << "b left\n"; //debug
	if (!instance.isFin())
		return false;
	instance.prepare();
// std::cout << ">> finished a packet[" << fd << "]:\n"; instance.print(false); //debug
	return true;
}

// returns true if execution was a success
// else fd was not found or request is unfinished
bool Router::executeRequest(int fd)
{
	std::map<int, Request>::iterator it = _requests.find(fd);
	if (it == _requests.end())
		return false;
	Request& instance = (*it).second;
	if (!instance.isFin())
		return false;
	instance.handle();
	deleteRequest(fd);
	return true;
}

void Router::deleteRequest(int fd)
{
	std::map<int, Request>::iterator it = _requests.find(fd);
	if (it != _requests.end())
		_requests.erase(it);
}
