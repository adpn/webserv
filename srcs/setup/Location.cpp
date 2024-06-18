#include <dirent.h>

#include "Location.hpp"
#include "Server.hpp"
#include "Entry.hpp"

//--------------- Orthodox Canonical Form ---------------//
Location::Location(Server& server)
	: _server(server) {
	this->_limit_except["GET"] = false;
	this->_limit_except["POST"] = false;
	this->_limit_except["DELETE"] = false;
	this->_autoindex = false;
}
Location::Location(const Location &other)
	: _server(other._server), _name(other._name), _limit_except(other._limit_except),
	_return(other._return), _root(other._root), _autoindex(other._autoindex), _index(other._index), _aliases(other._aliases) {}

Location::Location(const Location &other, Server& server)
	: _server(server), _name(other._name), _limit_except(other._limit_except),
	_return(other._return), _root(other._root), _autoindex(other._autoindex), _index(other._index), _aliases(other._aliases) {}

// THIS IS PRIVATE AND SHOULD NEVER BE USED
// ALSO DON'T DEFAULT CONSTRUCT
Location &Location::operator=(const Location &other){
	// this->_name = other._name;
	// this->_limit_except = other._limit_except;
	// this->_return = other._return;
	// this->_autoindex = other._autoindex;
	// this->_index = other._index;
	// this->_root = other._root;
	// this->_aliases = other._aliases;
	(void) other;
	return *this;
}
Location::~Location() {}



//--------------- Setters ---------------//
void	Location::set_name(std::string const& name){
	_name = name;
}

void	Location::set_limit_except(std::vector<std::string> s){
	std::string	authorized_method[3] = {"GET", "POST", "DELETE"};
	size_t i;

	for (size_t j = 0; j < s.size(); j++){
		for (i = 0; i < 4; i++){
			if (i == 3)
				throw Location::Error("Directive format not respected.");
			if (authorized_method[i] == s[j]){
				this->_limit_except[authorized_method[i]] = true;
				break ;
			}
		}
	}
}
void	Location::set_return(std::vector<std::string> s){
	std::string status;
	std::string path;

	if (s.size() != 2)
		throw Location::Error("Directive format not respected.");
	status = s.front();
	if (status.find_first_not_of("0123456789") != std::string::npos
			|| atof(status.c_str()) > 599)
		throw Location::Error("Directive format not respected.");
	this->_return = std::make_pair(atoi(s[0].c_str()), s[1]);
}
void	Location::set_autoindex(std::vector<std::string> s ){
	if (s.size() != 1)
		throw Location::Error("Directive format not respected.");
	std::string	authorized_value[2] = {"off", "on"};
	size_t i;
	for (i = 0; i < 2; i++){
		if (authorized_value[i] == s.front()){
			this->_autoindex = (i ? true : false);
			return ;
		}
	}
	throw Location::Error("Directive format not respected.");
}
void	Location::set_alias(std::vector< std::string > s){
	if (!s.size())
		throw Location::Error("Directive format not respected.");
	for (size_t i = 0; i < s.size(); ++i)
		_aliases.push_back(s[i]);
}
void	Location::set_index(std::vector< std::string > s){
	if (!s.size())
		throw Location::Error("Directive format not respected.");
	this->_index = s;
}
void	Location::set_root(std::vector< std::string > s){
	if (s.size() != 1)
		throw Location::Error("Directive format not respected.");
	this->_root = s.front();
}



//--------------- Getters ---------------//
Server const& Location::get_server() const {
	return _server;
}
std::string const& Location::get_name() const {
	return _name;
}
std::map<std::string, bool> const& Location::get_limit_except() const {
	return this->_limit_except;
}
std::pair<unsigned int, std::string> const& Location::get_return() const {
	return this->_return;
}
bool Location::get_autoindex() const {
	return this->_autoindex;
}
std::vector<std::string> const& Location::get_index() const {
	return this->_index;
}
std::list<std::string> const& Location::get_aliases() const {
	return this->_aliases;
}
// returns a path without any leading or trailing '/'
std::string	Location::get_root(bool print) const {
	if (print)
		return _root;
	string ret(_root);
	if (_root.empty())
		ret = _server.get_generic_root();
	if (ret.empty() || ret == "/")
		return ".";
	ret.erase(0, ret.find_first_not_of('/'));
	ret.erase(ret.find_last_not_of('/') + 1);
	return ret;
}



//--------------- Members ---------------//
bool Location::is_allowed(std::string const& method) const
{
	std::map<std::string, bool>::const_iterator it = _limit_except.find(method);
	if (it == _limit_except.end())
		return false;
// @Dennis does this seem right to you?
	return (*it).second;
}
void Location::aliases_to_server(Server& server)
{
	for (std::list<std::string>::const_iterator it = _aliases.begin(); it != _aliases.end(); ++it)
		server.set_alias(*it, *this);
	server.set_alias(_name, *this);
}

// throws if root can't be opened
std::vector<Entry> Location::create_entries(std::string uri) const {
	DIR* dirp = opendir((get_root() + uri).c_str());
	if (!dirp)
		throw Location::Error("couldn't open location root: " + (get_root() + uri));
	std::vector<Entry> res;
	struct dirent* entry;
	entry = readdir(dirp);
	while (entry)
	{
		res.push_back(Entry(entry));
		entry = readdir(dirp);
	}
	closedir(dirp);
	return res;
}



//--------------- Error management ---------------//
Location::Error::Error(std::string message) : _msg(message){}
Location::Error::~Error() throw(){};
const char *Location::Error::what() const throw(){
	return this->_msg.c_str();
}



//--------------- Output debug ---------------//
std::ostream& operator<<(std::ostream& o, Location const& l)
{
	o << "	location: " << l.get_name() << "\n";
	o << "		server: " << l.get_server().get_name().front() << "\n";
	o << "		root: " << l.get_root(true) << "\n";
	o << "		limits: \n";
	for (std::map<std::string, bool>::const_iterator it = l.get_limit_except().begin(); it != l.get_limit_except().end(); ++it)
		o << "			" << (*it).first << " " << std::boolalpha << (*it).second << "\n";
	o << "		return: " << l.get_return().first << " " << l.get_return().second << "\n";
	o << "		autoindex: " << std::boolalpha << l.get_autoindex() << "\n";
	o << "		index: \n";
	for (size_t i = 0; i < l.get_index().size(); ++i)
		o << "			" << l.get_index()[i] << "\n";
	o << std::endl;
	return o;
}
