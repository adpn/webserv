#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <vector>
# include <map>
# include <list>
# include <string>

class Server;
class Entry;

/*
Class Location
	private members:
		_limit_except[3] -> accepted_HTTP_methods(); default value : false
		_return;		 ->	HTTP_redirections
		_autoindex;		 ->	directory_listing, default value : false
		_index;			 ->	default_file

	Either _autoindex is true or/and _index is set
	_autoindex takes over if true

	/!\ missing /!\
		Make the route able to accept uploaded files and configure where they should be saved.
		Execute CGI based on certain file extension (for example .php).
*/
class Location {
	private:
		Server&									_server;
		std::string								_name;
		std::map<std::string, bool>				_limit_except;
		std::pair<unsigned int, std::string>	_return;
		std::string								_root;
		std::string								_upload_path;
		bool									_autoindex;
		std::vector<std::string>				_index;

		Location &operator=(const Location &other);
	public:
		//--- Orthodox Canonical Form ---//
		Location(Server& server);
		Location(const Location &other);
		Location(const Location &other, Server& server);
		~Location();

		//--- Setters ---//
		void	set_name(std::string const& name);
		void	set_limit_except(std::vector< std::string > rawString);
		void	set_return(std::vector< std::string > rawString);
		void	set_root(std::vector< std::string > rawString);
		void	set_autoindex(std::vector< std::string > rawString);
		void	set_index(std::vector< std::string > rawString);
		void	set_upload_path(std::vector< std::string > rawString);

		//--- Getters ---//
		Server const&								get_server() const;
		std::string const&							get_name() const;
		std::map<std::string, bool> const&			get_limit_except() const;
		std::pair<unsigned int, std::string> const&	get_return() const;
		std::string const&							get_root(bool debug = false) const;
		std::string const&							get_upload_path() const;
		bool										get_autoindex() const;
		std::vector<std::string> const&				get_index() const;

		//--- Members ---//
		bool				is_allowed(std::string const& method) const;
		std::vector<Entry>	create_entries(std::string uri) const;

		//--- Error management ---//
		class Error : public std::exception {
			private:
				std::string	_msg;
			public:
				Error(std::string message);
				virtual ~Error() throw();
				virtual const char *what() const throw();
		};
};

// debug
std::ostream& operator<<(std::ostream& o, Location const& l);

#endif
