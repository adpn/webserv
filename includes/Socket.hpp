#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h> 
# include <exception>

# define MAX_CLIENTS 2000

class Socket {
private:
	int _fd;
	int _port;

public:
	Socket(int port);
	Socket(const Socket& other);
	~Socket();
	int getFd() const;
	int getPort() const;

	operator int() const;

	class BoundFailException : std::exception {
	public:
		const char* what() const throw();
	};
	class ListenFailException : std::exception {
	public:
		const char* what() const throw();
	};
	class SocketFailException : std::exception {
	public:
		const char* what() const throw();
	};
};



#endif