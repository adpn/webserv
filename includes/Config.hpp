#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <fstream>
# include <string>
# include <vector>
# include <list>
# include <iterator>

# include "Server.hpp"

# define DELIMITERS	" 	"
# define NOTFOUND	std::string::npos

class Config {
	private:
		unsigned int				_brackets;
		std::ifstream				_fd;
		std::vector<std::string> 	_example_server_bloc;
		std::list<Server>			_servers;

	public:
		//--- Orthodox Canonical Form ---//
		Config(std::string filename);
		~Config();

		//--- Configuration ---//
		void	bufferize();
		size_t	count_brackets(std::string buffer);

		//--- Server ---//
		void	add_server(std::string server_block);
		bool	server_approved(Server const& server);

		std::list<Server>& get_servers();

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

std::vector< std::string > tokenizer( std::string string, std::string delimiters);

#endif