#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

# include <vector>
# include <map>
# include <string>

/*
Class Location
	private members:
		_limit_except[3] -> accepted_HTTP_methods(); default value : true
		_return;		 ->	HTTP_redirections
		_alias;			 ->	directory_to_search
		_autoindex;		 ->	directory_listing, default value : false
		_index;			 ->	default_file

	/!\ missing /!\ 
		Make the route able to accept uploaded files and configure where they should be saved.
		Execute CGI based on certain file extension (for example .php).
*/
class Location {
	private:
		std::map<std::string, bool>						_limit_except;
		std::pair<unsigned int, std::string>			_return;
		std::vector<std::string>						_alias;
		bool											_autoindex;
		std::vector<std::string>						_index;
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
		void	set_autoindex(std::vector< std::string > rawString);
		void	set_index(std::vector< std::string > rawString);

		//--- Getters ---//
		std::map<std::string, bool>				get_limit_except();
		std::pair<unsigned int, std::string>	get_return();
		std::vector<std::string>				get_alias();
		bool									get_autoindex();
		std::vector<std::string>				get_index();

		//--- Error management ---//
		class DirectiveError : public std::exception {
			public:
				virtual const char *what() const throw();
		} ;
};

std::ostream& operator<<( std::ostream& o, Location & location);

#endif