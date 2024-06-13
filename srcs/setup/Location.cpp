#include <dirent.h>

#include "Location.hpp"
#include "Entry.hpp"

//--------------- Orthodox Canonical Form ---------------//
Location::Location(){
	this->_limit_except["GET"] = false;
	this->_limit_except["POST"] = false;
	this->_limit_except["DELETE"] = false;
	this->_autoindex = false;
}
Location::Location(const Location &other){
	*this = other;
}
Location &Location::operator=(const Location &other){
	this->_limit_except = other._limit_except;
	this->_return = other._return;
	this->_alias = other._alias;
	this->_autoindex = other._autoindex;
	this->_index = other._index;
	this->_root = other._root;
	return *this;
}
Location::~Location(){}



//--------------- Setters ---------------//
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
	this->_alias = s;
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
std::map<std::string, bool> Location::get_limit_except(){
	return this->_limit_except;
}
std::pair<unsigned int, std::string> Location::get_return(){
	return this->_return;
}
std::vector<std::string> Location::get_alias(){
	return this->_alias;
}
bool Location::get_autoindex(){
	return this->_autoindex;
}
std::vector<std::string> Location::get_index(){
	return this->_index;
}
std::string	Location::get_root(){
	return this->_root;
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

std::string Location::full_root() const {
	// DO STUFF HERE WHEN WE GET THE ROOT VARIABLE
	return std::string();
}

std::vector<Entry> Location::create_entries() const {
	DIR* dirp = opendir(full_root().c_str());
	if (!dirp)
		throw Location::Error("couldn't open location root: " + full_root());
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
std::ostream& operator<<( std::ostream& o, Location& location){
	std::string methods[3] = {"GET", "POST", "DELETE"};
	o << "	";
	for (int i = 0; i < 3; i++){
		o << methods[i] << " : " << std::boolalpha << location.get_limit_except()[methods[i]] << " ";
	}
	o << std::endl;
	o << "	return : " << location.get_return().first << " " << location.get_return().second << std::endl;
	o << "	root : " << location.get_root() << std::endl;
	o << "	alias :";
	for (size_t i = 0; i < location.get_alias().size(); i++){
		o << " " << location.get_alias()[i];
	}
	o << std::endl;
	o << "	autoindex : " << location.get_autoindex() << std::endl;
	o << "	index : ";
	for (size_t i = 0; i < location.get_index().size(); i++){
		o << " " << location.get_index()[i];
	}
	o << std::endl;
	return o;
}