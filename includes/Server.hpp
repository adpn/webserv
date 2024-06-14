#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <vector>
# include <list>
# include <string>
# include <fstream>
# include <iostream>

class Location;
class Socket;

# define NOTFOUND	std::string::npos

class Server {
	private:
		std::vector<unsigned int>			_port;
		std::pair<unsigned int, char>		_request_size;
		std::vector<std::string>			_name;
		std::vector<Socket>					_sockets;
		std::map<unsigned int, std::string>	_error_page;
		std::list<Location>					_locations;
		std::string							_generic_root;
		std::map<std::string, Location&>	_aliases;

		Server & operator=(const Server & other);
	public:
		//--- Orthodox Canonical Form ---//
		Server();
		Server(const Server & other);
		~Server();

		//--- Setters ---//
		void	set_port( std::vector< std::string > s );
		void	set_request_size( std::vector< std::string > s );
		void	set_name( std::vector< std::string > s );
		void	set_error_page( std::vector< std::string > s );
		void	set_generic_root( std::vector< std::string > s );
		void	set_location( Location const& loc_block);
		void	set_alias( std::string const& s, Location& loc);
		void	initSockets();

		//--- Getters ---//
		std::vector<std::string> const&				get_name() const;
		std::pair<unsigned int, char> const&		get_request_size() const;
		std::vector<unsigned int> const&			get_port() const;
		std::map<unsigned int, std::string> const&	get_error_page() const;
		std::string const&							get_generic_root() const;
		std::list<Location> const&					get_locations() const;
		std::map<std::string, Location&> const&		get_aliases() const;
		std::vector<Socket>&						get_sockets();

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
std::ostream& operator<<( std::ostream& o, Server const& S);
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p) {
    os << p.first << " -> " << p.second;
    return os;
}

#endif
