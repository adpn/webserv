#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <vector>
# include <string>
# include <fstream>
# include <iostream>

# include "Location.hpp"
# include "Socket.hpp"

# define NOTFOUND	std::string::npos

class Server {
	private:
		std::vector<unsigned int>				_port;
		std::pair<unsigned int, char>			_request_size;
		std::vector<std::string>				_name;
		std::vector<Socket>						_sockets;
		std::map<unsigned int, std::string>		_error_page;
		// std::string								_generic_root;
		std::map<std::string, Location>			_location;
	public:
		//--- Orthodox Canonical Form ---//
		Server();
		Server(const Server & other);
		Server & operator=(const Server & other);
		~Server();
	
		//--- Setters ---//
		void	set_port( std::vector< std::string > s );
		void	set_request_size( std::vector< std::string > s );
		void	set_name( std::vector< std::string > s );
		void	set_error_page( std::vector< std::string > s );
		// void	set_generic_root( std::vector< std::string > s );
		void	set_location( std::string s, Location loc_block);
		void	initSockets();

		//--- Getters ---//
		std::vector<std::string>			get_name();
		std::pair<unsigned int, char>		get_request_size();
		std::vector<unsigned int>			get_port();
		std::map<unsigned int, std::string>	get_error_page();
		// std::string							get_generic_root();
		std::map<std::string, Location>		get_location();
		std::vector<Socket>&				get_sockets();

		void	closeSockets();

		//--- Error management ---//
		class Error : public std::exception {
			private:
				std::string	_msg;
			public:
				Error(std::string message);
				virtual ~Error() throw();
				virtual const char *what() const throw();
		} ;

} ;


// debug only
std::ostream& operator<<( std::ostream& o, Server & S);
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p) {
    os << p.first << " -> " << p.second;
    return os;
}


#endif