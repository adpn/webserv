#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <vector>
# include <string>
# include <fstream>
# include <iostream>

# include "Location.hpp"

# define NOTFOUND	std::string::npos

class Server {
	private:
		std::vector<unsigned int>				_port;
		std::pair<unsigned int, char>			_request_size;
		std::vector<std::string>				_name;
		std::map<unsigned int, std::string>		_error_page;
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
		void	set_location( std::string s, Location loc_block);

		//--- Getters ---//
		std::vector<std::string>			get_name();
		std::pair<unsigned int, char>		get_request_size();
		std::vector<unsigned int>			get_port();
		std::map<unsigned int, std::string>	get_error_page();
		std::map<std::string, Location>		get_location();

		class PortError : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class RequestedSizeError : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class NameError : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class ErrorPageError : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;

} ;


std::ostream& operator<<( std::ostream& o, Server & S);

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p) {
    os << p.first << " -> " << p.second;
    return os;
}


#endif