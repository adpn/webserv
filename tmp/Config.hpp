#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <fstream>
# include <string>
# include <vector>
# include <iterator>

# include "Server.hpp"

# define DELIMITERS	" 	"
# define NOTFOUND	std::string::npos

class Config {
	private:
		unsigned int				_brackets;
		std::ifstream				_fd;
		std::vector<std::string> 	_example_server_bloc;

		std::vector<Server>			_servers;

		// Config();
		// Config(const Config &other);
		// Config &operator=(const Config &other);
	public:
		//--- Orthodox Canonical Form ---//
		Config(std::string filename);
		~Config();

		//--- Configuration ---//
		void	bufferize();
		size_t	count_brackets(std::string buffer);
		
		//--- Server ---//
		void	add_server(std::string server_block);
		bool	server_approved(Server server);

		std::vector<Server> get_servers();

		//--- Error management ---//
		// class ConfigError : public std::exception {
		// 	public:
		// 		virtual const char *Error(char *str) const throw();
		// } ;
		class FailToOpen : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class DirectiveError : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class FoundWeirdStuff : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class NoServerFound : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class CommonPort : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class DataMissing : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class BracketError : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class WrongMethod : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
		class LocationError : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
} ;

std::vector< std::string > tokenizer( std::string string, std::string delimiters);

#endif