#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"

//--------------- Orthodox Canonical Form ---------------//
Config::Config( std::string filename ) : _brackets(0), _fd(filename.c_str()) {
	if (!_fd)
		throw Config::Error("Open input file failed.");
	this->_example_server_bloc.push_back("server");
	this->_example_server_bloc.push_back("{");
	bufferize();
};
Config::~Config() {
	this->_fd.close();
};


//--------------- Getters ---------------//
std::list<Server>& Config::get_servers() {
	return this->_servers;
}


//--------------- Error management ---------------//
Config::Error::Error(std::string message) : _msg(message) {}
Config::Error::~Error() throw() {};
const char *Config::Error::what() const throw() {
	return this->_msg.c_str();
}


//--------------- Location ---------------//
void	add_location_directive(Location &LocationBlock, std::string loc_dir) {
	std::string	DirectivesName[7] = {"limit_except",
										"return",
										"autoindex",
										"index",
										"root",
										"upload_path"};
	void (Location::*DirectivesFonction[7])
		(std::vector< std::string > rawString) = {&Location::set_limit_except,
											&Location::set_return,
											&Location::set_autoindex,
											&Location::set_index,
											&Location::set_root,
											&Location::set_upload_path};

	std::vector< std::string > VectorDirective = tokenizer(loc_dir, DELIMITERS);
	if (VectorDirective.size() < 1)
		throw Config::Error("Directive format not respected (" + LocationBlock.get_name() + ")");
	for (int i = 0; i < 7; i++) {
		if (DirectivesName[i] == VectorDirective[0]) {
			VectorDirective.erase(VectorDirective.begin());
			(LocationBlock.*DirectivesFonction[i])(VectorDirective);
			return ;
		}
	}
	throw Config::Error("Directive format not respected (" + LocationBlock.get_name() + ")");
}
void	add_location(Server & server, std::string RawStr) {
	//	keep location path
	Location 					LocationBlock(server);
	size_t 						next_sep;
	size_t 						FirstBracket = RawStr.find_first_of("{");
	std::vector<std::string>	VectorDirectives = tokenizer(RawStr.substr(0, FirstBracket), DELIMITERS);

	if (VectorDirectives.size() != 2 || VectorDirectives.front() != "location" || VectorDirectives[1].empty())
		throw Config::Error("Directive format not respected.");
	LocationBlock.set_name(VectorDirectives[1]);
	RawStr.erase(0, FirstBracket + 1);

	while (RawStr.size()) {
		next_sep = RawStr.find_first_of(";}");
		switch (RawStr[next_sep]) {
			case ';':
				add_location_directive(LocationBlock, RawStr.substr(0, next_sep));
				RawStr.erase(0, next_sep + 1);
				break ;
			case '}':
				if (RawStr.find_first_not_of(" 	}") != NOTFOUND)
					throw Config::Error("Found weird stuff..");
				server.set_location(LocationBlock);
				return ;
		}
	}
}


//--------------- Server ---------------//
void	add_server_directive(Server & server, std::string directive) {
	std::string	directives_name[5] = {"listen",
									"client_max_body_size",
									"server_name",
									"error_page",
									"generic_root"};
	void (Server::*directives_fonction[5])
		(std::vector< std::string > s) = {&Server::set_port,
											&Server::set_request_size,
											&Server::set_name,
											&Server::set_error_page,
											&Server::set_generic_root};


	std::vector< std::string > vect_dir	= tokenizer(directive, DELIMITERS);
	if (vect_dir.size() < 1)
		throw Config::Error("Directive format not respected.");
	for (int i = 0; i < 5; i++) {
		if (directives_name[i] == vect_dir[0]) {
			vect_dir.erase(vect_dir.begin());
			(server.*directives_fonction[i])( vect_dir );
			return ;
		}
	}
	throw Config::Error("Directive format not respected.");
}
bool	Config::server_approved(Server const& server) {
	std::vector<unsigned int> open_ports;
	std::vector<unsigned int> nw_ports = server.get_port();

	if (!server.get_port().size())
		throw Config::Error("Data's missing.");
	if (server.get_generic_root().empty())
		for (std::list<Location>::const_iterator it = server.get_locations().begin(); it != server.get_locations().end(); ++it)
			if (it->get_root(true).empty())
				throw Config::Error("No root for location \"" + it->get_name() + "\"");
	for (std::list<Server>::const_iterator it = _servers.begin(); it != _servers.end(); ++it) {
		open_ports = it->get_port();
		for (size_t j = 0; j < open_ports.size(); j++)
			for (size_t m = 0; m < nw_ports.size(); m++)
				if (open_ports[j] == nw_ports[m])
					std::cout << "WARNING: double port on " << open_ports[j] << std::endl;
	}
	return true;
}
void	Config::add_server(std::string server_block) {
	size_t	next_sep;
	size_t	stop_location;
	Server	server;

	while (server_block.size()) {
		next_sep = server_block.find_first_of(";{}");
		switch (server_block[next_sep]) {
			case ';':	// directive
				add_server_directive(server, server_block.substr(0, next_sep));
				server_block.erase(0, next_sep + 1);
				break;
			case '{':	// location
				stop_location = server_block.find_first_of("}");
				try {
					add_location(server, server_block.substr(0, stop_location + 1));
				}
				catch (std::exception &e) {
					std::cout << "Location block rejected: " << e.what() << std::endl;
				}
				server_block.erase(0, stop_location + 1);
				break;
			case '}':	// end of bloc server
				if (server_block.find_first_not_of(" 	}") != NOTFOUND)
					throw Config::Error("Found weird stuff..");
				if (server_approved(server)) {
					this->_servers.push_back(server);
					return ;
				}
				else
					throw Config::Error("Server not approved");
		}
	}
}


//--------------- Configuration ---------------//
std::vector< std::string > tokenizer( std::string string, std::string delimiters) {
	int							ln_w = 0;
	std::vector< std::string >	vector;

	int i;
	for (i = 0; string[i]; i++) {
		if ( delimiters.find_first_of( string[i] ) != NOTFOUND ) {
			if (ln_w) {
				vector.push_back(string.substr(i - ln_w, ln_w));
				ln_w = 0;
			}
		}
		else
			ln_w += 1;
	}
	if (ln_w)
		vector.push_back(string.substr( i - ln_w, ln_w ));
	return vector;
}
size_t	Config::count_brackets(std::string line) {
	for (int i = 0; line[i]; i++) {
		switch (line[i]) {
			case '{':
				if (this->_brackets > 2)
					throw Config::Error("Something went wrong with a bracket.");
				this->_brackets++;
				break;
			case '}':
				this->_brackets--;
				if (!this->_brackets)
					return i + 1;	// keep the server's closing bracket
				break;
		}
	}
	return line.size();
}
void	Config::bufferize() {
	std::string buffer;
	std::string	server_block;
	size_t		end;

	while (getline(this->_fd, buffer)) {
		if (_brackets) { // add new line to current server content
			end = count_brackets(buffer);
			if (buffer.find_first_of('#') < end) // ignore comment
				end = buffer.find_first_of('#');
			server_block.append(buffer.c_str(), end);
		}
		if (!_brackets && server_block.size()) {	// if server content not empty, add it
			try {
				add_server(server_block);
				std::cout << this->_servers.back() << std::endl;
			}
			catch (std::exception & e) {
				std::cout << "Server block rejected: " << e.what() << std::endl;
			}
			server_block.clear();
		}
		if (tokenizer(buffer, DELIMITERS) == this->_example_server_bloc) // looking for a new server to start
			_brackets++;
	}
	if (this->_servers.empty())
		throw Config::Error("No server found.");
}


//--------------- Processing ---------------//

// removes any leading and trailing '/', any leading '~' and simulates '..'
std::string Config::process_path(std::string const& input)
{
	std::string ret(input);
	ret.erase(0, ret.find_first_not_of("/~"));
	ret.erase(ret.find_last_not_of('/') + 1);
	size_t ddot_pos = ret.find("/../");
	if (ddot_pos == std::string::npos && ret.size() >= 3 && !ret.compare(ret.size() - 3, 3, "/.."))
		ddot_pos = ret.size() - 3;
	while (ddot_pos != std::string::npos)
	{
		size_t last_slash_pos = std::string::npos;
		if (ddot_pos)
			last_slash_pos = ret.rfind('/', ddot_pos - 1);
		if (last_slash_pos == std::string::npos)
		{
			last_slash_pos = 0;
			ddot_pos++;
		}
		ret.erase(last_slash_pos, ddot_pos + 3 - last_slash_pos);
		ddot_pos = ret.find("/../", last_slash_pos);
		if (ddot_pos == std::string::npos && ret.size() >= 3 && !ret.compare(ret.size() - 3, 3, "/.."))
			ddot_pos = ret.size() - 3;
	}
	if (ret.size() >= 3 && !ret.compare(0, 3, "../"))
		ret.erase(0, 3);
	if (ret == "..")
		return std::string();
	return ret;
}
