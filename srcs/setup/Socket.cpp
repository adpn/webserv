#include <iostream>

#include "Socket.hpp"

/* Exceptions */

const char* Socket::BoundFailException::what() const throw() {
	return "socket bound fail";
}

const char* Socket::ListenFailException::what() const throw() {
	return "socket listen fail";
}

const char* Socket::SocketFailException::what() const throw() {
	return "socket creation fail";
}

/* Member functions */

Socket::Socket(int port) : _port(port) {
	/*
	AF_INET for IPV4
	SOCK_STREAM for TCP
	0 for default TCP
	*/
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw SocketFailException();

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET; //IPV4 again
	serverAddr.sin_addr.s_addr = INADDR_ANY; //take all IP
	serverAddr.sin_port = htons(port); //which port we will listen to

	//Link socket with settings
	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1)
		throw BoundFailException();

	std::cout << "Created socket " << _fd;
	std::cout << " with port: " << _port << std::endl;
	// Listen to socket connections
	if (listen(_fd, MAX_CLIENTS) == -1)
		throw ListenFailException();
}

Socket::Socket(const Socket& other) : _port(other._port) {
	_fd = other._fd;
}

Socket::operator int() const {
	return _fd;
}

int Socket::getFd() const {
	return _fd;
}

int Socket::getPort() const {
	return _port;
}
