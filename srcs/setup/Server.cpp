#include "Server.hpp"
#include "Location.hpp"

//--------------- Orthodox Canonical Form ---------------//
// default _request_size
Server::Server() : _request_size(100, 'M') {
}
Server::Server( const Server & other ) {
	for (std::list<Location>::const_iterator it = other.get_locations().begin(); it != other.get_locations().end(); ++it)
		_locations.push_back(Location(*it, *this));
	for (std::map<std::string, Location&>::const_iterator it_alias = other._aliases.begin(); it_alias != other._aliases.end(); ++it_alias)
	{
		std::list<Location>::iterator it_loc;
		for (it_loc = _locations.begin(); it_loc != _locations.end(); ++it_loc)
			if (it_loc->get_name() == it_alias->second.get_name())
				break ;
		if (it_loc == _locations.end())
		{
			std::cout << "couldnt find |" << it_alias->second.get_name() << "| in server copy\n";
			throw Server::Error("something went wrong copying a server instance");
		}
		_aliases.insert(std::pair<std::string, Location&>(it_alias->first, *it_loc));
	}
	*this = other;
}
// THIS IS PRIVATE AND SHOULD NEVER BE USED
Server & Server::operator=(const Server & other){
	_port = other._port;
	_request_size = other._request_size;
	_name = other._name;
	_error_page = other._error_page;
	_generic_root = other._generic_root;
	// THESE USE REFERENCES
	// _locations = other._locations;
	// _aliases = other._aliases;
	return (*this);
}
Server::~Server(){}


//--------------- Error management ---------------//
Server::Error::Error(std::string message) : _msg(message){}
Server::Error::~Error() throw(){};
const char *Server::Error::what() const throw(){
	return this->_msg.c_str();
}


//--------------- Setters ---------------//
void	Server::set_port( std::vector< std::string > s ){
	for (size_t i = 0; i < s.size(); i++){
		if (s[i].find_first_not_of("0123456789") != NOTFOUND
			|| atof(s[i].c_str()) > 65535)
			throw Server::Error("Port format invalid.");
		// common port
		for (size_t j = 0; j < this->_port.size(); j++){
			if (this->_port[j] == static_cast<unsigned int>(atoi(s[i].c_str())))
				return ;
		}
		this->_port.push_back(atoi(s[i].c_str()));
	}
}
void	Server::set_request_size( std::vector< std::string > s ){
	std::string	authorized_units = "KM";
	if (s.size() > 1
		|| atof(s.front().c_str()) > INT_MAX
		|| s.front().find_first_not_of("0123456789") != s.front().size() - 1
		|| authorized_units.find_first_of(s[0].back()) == NOTFOUND)
		throw Server::Error("Request size format invalid.");
	this->_request_size.first = atoi(s.front().c_str());
	this->_request_size.second = s[0].back();
}
void	Server::set_name( std::vector< std::string > s ){
	if (!s.size())
		throw Server::Error("No name found.");
	for (size_t i = 0; i < s.size(); i++)
		this->_name.push_back(s[i]);
}
void	Server::set_error_page( std::vector< std::string > s ){
	if (s.size() < 2)
		throw Server::Error("No error page found");
	for (size_t i = 0; i < s.size() - 1; i++){
		if (s.size() < 2
			|| s[i].find_first_not_of("0123456789") != NOTFOUND
			|| atof(s[i].c_str()) > 599)
			throw Server::Error("Error page number invalid.");
		this->_error_page[atoi(s[i].c_str())] = s.back();
	}
}
void	Server::set_location( Location const& loc_block){
	_locations.push_back(loc_block);
	_locations.back().aliases_to_server(*this);
}
// overwrites if s already exists
void	Server::set_alias( std::string const& s, Location& loc){
	std::map<std::string, Location&>::iterator it = _aliases.find(s);
	if (it != _aliases.end())
		_aliases.erase(it);
	_aliases.insert(std::pair<std::string, Location&>(s, loc));
}
void	Server::set_generic_root( std::vector< std::string > s ){
	if (s.size() != 1)
		throw Server::Error("Generic root invalid.");
	this->_generic_root = s.front();
}

//--------------- Getters ---------------//
std::vector<unsigned int> const& Server::get_port() const {
	return this->_port;
}
std::pair<unsigned int, char> const& Server::get_request_size() const {
	return this->_request_size;
}
std::vector<std::string> const& Server::get_name() const {
	return this->_name;
}
std::map<unsigned int, std::string> const& Server::get_error_page() const {
	return this->_error_page;
}
std::string const& Server::get_generic_root() const {
	return this->_generic_root;
}
std::list<Location>	const& Server::get_locations() const {
	return this->_locations;
}
std::map<std::string, Location&> const& Server::get_aliases() const {
	return _aliases;
}

//--------------- Output debug ---------------//
std::ostream& operator<<( std::ostream& o, Server const& S){
	std::vector<std::string> names = S.get_name();
	o << "Name : ";
	for (std::vector< std::string >::iterator it = names.begin(); it != names.end(); it++){
		o << *it;
		if (it + 1 != names.end())
			o << ", ";
	}
	std::vector< unsigned int >	port = S.get_port();
	o << "\nport : ";
	for (std::vector< unsigned int >::iterator it = port.begin(); it != port.end(); it++){
		o << *it;
		if (it + 1 != port.end())
			o << ", ";
	}
	o << "\n" << "Request size : " << S.get_request_size().first << S.get_request_size().second << "\n";
	o << "Error page :\n";
	std::map<unsigned int, std::string>	errors_page = S.get_error_page();
	for (std::map<unsigned int, std::string>::iterator it = errors_page.begin(); it != errors_page.end(); it++){
		o << "	" << *it << "\n";
	}
	o << "Generic root : " << S.get_generic_root() << "\n";
	std::list<Location> const& location_vec = S.get_locations();
	o << "Locations(" << location_vec.size() << ") :\n";
	for (std::list<Location>::const_iterator it = location_vec.begin(); it != location_vec.end(); ++it){
		o << *it;
	}
	o << "aliases :\n";
	for (std::map<std::string, Location&>::const_iterator it = S.get_aliases().begin(); it != S.get_aliases().end(); ++it){
		o << "	" << (*it).first << " ~ " << (*it).second.get_name() << "\n";
	}
	o << std::endl;
	return o;
}
