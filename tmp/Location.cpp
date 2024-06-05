#include "Location.hpp"

//--------------- Orthodox Canonical Form ---------------//
Location::Location(){
	this->_limit_except["GET"] = true;
	this->_limit_except["POST"] = true;
	this->_limit_except["DELETE"] = true;
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
	return *this;
}
Location::~Location(){}



//--------------- Setters ---------------//
void	Location::set_limit_except(std::vector<std::string> s){
	// std::cout << "Location configuration :";
	// for (size_t i = 0; i < s.size(); i++){
	// 	std::cout << " " << s[i];
	// }
	std::cout << std::endl;
	std::string	authorized_method[3] = {"GET", "POST", "DELETE"};
	size_t i;
	
	for (size_t j = 0; j < s.size(); j++){
		for (i = 0; i < 4; i++){
			if (i == 3)
				throw Location::DirectiveError();
			if (authorized_method[i] == s[j]){
				this->_limit_except[authorized_method[i]] = false;
				break ;
			}
		}
	}
}
void	Location::set_return(std::vector<std::string> s){
	// std::cout << "Location configuration :";
	// for (size_t i = 0; i < s.size(); i++){
	// 	std::cout << " " << s[i];
	// }
	std::cout << std::endl;
	std::string status;
	std::string path;

	if (s.size() != 2)
		throw Location::DirectiveError();
	status = s.front();
	if (status.find_first_not_of("0123456789") != std::string::npos
			|| atof(status.c_str()) > 599)
		throw Location::DirectiveError();
	this->_return = std::make_pair(atoi(s[0].c_str()), s[1]);
}
void	Location::set_autoindex(std::vector<std::string> s ){
	// std::cout << "Location configuration :";
	// for (size_t i = 0; i < s.size(); i++){
	// 	std::cout << " " << s[i];
	// }
	std::cout << std::endl;
	if (s.size() != 1)
		throw Location::DirectiveError();
	std::string	authorized_value[2] = {"off", "on"};
	size_t i;
	for (i = 0; i < 2; i++){
		if (authorized_value[i] == s.front()){
			this->_autoindex = (i ? true : false);
			return ;
		}
	}
	throw Location::DirectiveError();
}
void	Location::set_alias(std::vector< std::string > s){
	// std::cout << "Location configuration :";
	// for (size_t i = 0; i < s.size(); i++){
	// 	std::cout << " " << s[i];
	// }
	std::cout << std::endl;
	if (!s.size())
		throw Location::DirectiveError();
	this->_alias = s;
}
void	Location::set_index(std::vector< std::string > s){
	// std::cout << "Location configuration :";
	// for (size_t i = 0; i < s.size(); i++){
	// 	std::cout << " " << s[i];
	// }
	std::cout << std::endl;
	if (!s.size())
		throw Location::DirectiveError();
	this->_index = s;
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

//--------------- Error management ---------------//
const char *Location::DirectiveError::what() const throw(){
	return "Directive format not respected.";
}


// std::ostream& operator<<(std::ostream& o, Location& S){
// 	o << "Location :";
// 	std::string methods[3] = {"GET", "POST", "DELETE"};
// 	for (size_t i = 0; i < 3; i++){
// 		o << "\n	" << methods[i] << " : " << S.get_limit_except()[methods[i]];
// 	}
// 	return o;
// }