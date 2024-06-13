#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <vector>
# include <map>
# include <string>

class Entry;

/*
Class Location
	private members:
		_limit_except[3] -> accepted_HTTP_methods(); default value : false
		_return;		 ->	HTTP_redirections
		_alias;			 ->	directory_to_search
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
		std::map<std::string, bool>				_limit_except;
		std::pair<unsigned int, std::string>	_return;
		std::vector<std::string>				_alias;
		std::string								_root;
		bool									_autoindex;
		std::vector<std::string>				_index;

	public:
		//--- Orthodox Canonical Form ---//
		Location();
		Location(const Location &other);
		Location &operator=(const Location &other);
		~Location();

		//--- Setters ---//
		void	set_limit_except(std::vector< std::string > rawString);
		void	set_return(std::vector< std::string > rawString);
		void	set_alias(std::vector< std::string > rawString);
		void	set_root(std::vector< std::string > rawString);
		void	set_autoindex(std::vector< std::string > rawString);
		void	set_index(std::vector< std::string > rawString);

		//--- Getters ---//
		std::map<std::string, bool>				get_limit_except();
		std::pair<unsigned int, std::string>	get_return();
		std::vector<std::string>				get_alias();
		bool									get_autoindex();
		std::string								get_root();
		std::vector<std::string>				get_index();

		//--- Members ---//
		bool				is_allowed(std::string const& method) const;
		std::string			full_root() const;
		std::vector<Entry>	create_entries() const;

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

std::ostream& operator<<( std::ostream& o, Location& location);

#endif
