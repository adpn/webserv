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

		void	add_server(std::string server_block);
		bool	server_approved(Server server);
		size_t	count_brackets(std::string buffer);
		void	bufferize();

	public:
		Config(std::string filename);
		~Config();
		std::vector<Server>& get_servers();

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