#include "Server.hpp"

//--------------- Orthodox Canonical Form ---------------//
Server::Server() : _request_size(100, 'M') {
}
Server::Server( const Server & other ){
	*this = other;
}
Server & Server::operator=(const Server & other){
	_port = other._port;
	_request_size = other._request_size;
	_name = other._name;
	_error_page = other._error_page;
	_location = other._location;
	return (*this);
}
Server::~Server(){}


//--------------- Error management ---------------//
const char *Server::PortError::what() const throw(){
	return "Port format not respected.";
}
const char *Server::RequestedSizeError::what() const throw(){
	return "Requested size format not respected.";
}
const char *Server::NameError::what() const throw(){
	return "Name format not respected.";
}
const char *Server::ErrorPageError::what() const throw(){
	return "Error page format not respected.";
}


//--------------- Setters ---------------//
void	Server::set_port( std::vector< std::string > s ){
	for (size_t i = 0; i < s.size(); i++){
		if (s[i].find_first_not_of("0123456789") != NOTFOUND
			|| atof(s[i].c_str()) > 65535)
			throw Server::PortError();
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
		throw Server::RequestedSizeError();
	this->_request_size.first = atoi(s.front().c_str());
	this->_request_size.second = s[0].back();
}
void	Server::set_name( std::vector< std::string > s ){
	if (!s.size())
		throw Server::NameError();
	for (size_t i = 0; i < s.size(); i++)
		this->_name.push_back(s[i]);
}
void	Server::set_error_page( std::vector< std::string > s ){
	if (s.size() < 2)
		throw Server::ErrorPageError();
	for (size_t i = 0; i < s.size() - 1; i++){
		if (s.size() < 2
			|| s[i].find_first_not_of("0123456789") != NOTFOUND
			|| atof(s[i].c_str()) > 599)
			throw Server::ErrorPageError();
		this->_error_page[atoi(s[i].c_str())] = s.back();
	}
}
void	Server::set_location( std::string s, Location loc_block){
	this->_location[s] = loc_block;
}


//--------------- Getters ---------------//
std::vector<unsigned int> Server::get_port(){
	return this->_port;
}
std::pair<unsigned int, char> Server::get_request_size(){
	return this->_request_size;
}
std::vector<std::string> Server::get_name(){
	return this->_name;
}
std::map<unsigned int, std::string> Server::get_error_page(){
	return this->_error_page;
}
std::map<std::string, Location>	Server::get_location(){
	return this->_location;
}


//--------------- Output debug ---------------//
std::ostream& operator<<( std::ostream& o, Server & S){
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
	o << "\n" << "Request size : " << S.get_request_size().first << S.get_request_size().second << "\nError page :\n";
	std::map<unsigned int, std::string>	errors_page = S.get_error_page();
	for (std::map<unsigned int, std::string>::iterator it = errors_page.begin(); it != errors_page.end(); it++){
		o << "	" << *it;
	}
	std::map<std::string, Location> location_map = S.get_location(); 
	for (std::map<std::string, Location>::iterator it = location_map.begin(); it != location_map.end(); it++){
		o << *it << std::endl;
	}
	o << std::endl;
	return o;
}
