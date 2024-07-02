#include "Response.hpp"
#include "Request.hpp"
#include "Location.hpp"
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <ctime>

/* Exceptions*/
const char* Response::SendFailException::what() const throw() {
	return "send failed";
}

/* CONSTRUCTORS */

Response::Response()
	: _version("HTTP/1.1"), _status(500), _reason("Internal Server Error"), _location(NULL)
	{ setHDate(); }

Response::Response(Request const& request)
	: _version("HTTP/1.1"), _status(request.getStatus()), _reason("Internal Server Error"), _location(NULL)
	{ setHDate(); }

Response::Response(Response const& src)
	{ this->operator=(src); }

Response::~Response()
	{}

Response& Response::operator=(Response const& rhs)
{
	_version = rhs._version;
	_status = rhs._status;
	_reason = rhs._reason;
	_fields = rhs._fields;
	_body = rhs._body;
	return *this;
}

/* G(/S)ETTERS */

bool Response::isGood() const
	{ return _status < 300; }

string const& Response::getReason() const
	{ return _reason; }

void Response::setLocation(Location const* location)
	{ _location = location; }

// sets 'reason' if available in internal set
bool Response::setStatus(int status)
{
	_status = 500;
	_reason = "Internal Server Error";
	if (status == 500)
		return true;
	if (status < 100 || status > 599)
		return false;
	std::pair<int, string> reasons[] = {
		std::pair<int, string>(100, "Continue"),
		std::pair<int, string>(200, "OK"),
		std::pair<int, string>(201, "Created"),
		std::pair<int, string>(204, "No Content"),
		std::pair<int, string>(301, "Moved Permanently"),
		std::pair<int, string>(302, "Found"),
		std::pair<int, string>(307, "Temporary Redirect"),
		std::pair<int, string>(308, "Permanent Redirect"),
		std::pair<int, string>(400, "Bad Request"),
		std::pair<int, string>(403, "Forbidden"),
		std::pair<int, string>(404, "Not Found"),
		std::pair<int, string>(405, "Method Not Allowed"),
		std::pair<int, string>(411, "Length Required"),
		std::pair<int, string>(413, "Content Too Large"),
		std::pair<int, string>(505, "HTTP Version Not Supported"),
		std::pair<int, string>(0, "")};
	for (int i = 0; reasons[i].first; ++i)
	{
		if (reasons[i].first != status)
			continue ;
		_status = status;
		_reason = reasons[i].second;
		return true;
	}
	return false;
}

void Response::setCustomReason(string const& reason)
{ _reason = reason; }

bool Response::setField(string const& field)
{
	if (field.find(':') == string::npos)
		return false;
	std::pair<string, string> pair;
	std::istringstream iss(field);

	std::getline(iss, pair.first, ':');
	iss >> std::ws;
	while (!iss.eof())
	{
		std::getline(iss, pair.second, ',');
		iss >> std::ws;
		if (pair.second.empty())
			continue ;
		std::pair<std::map<string, string>::iterator, bool> ret(_fields.insert(pair));
		if (!ret.second)
			if (ret.first->second.find(pair.second) == string::npos)
				ret.first->second.append("," + pair.second);
	}
	return true;
}

void Response::setBody(string const& body)
{
	_body = body;
}

string Response::findContentType(string file) {
	size_t dotIndex = file.find_last_of(".");
	if (dotIndex == string::npos)
		return "application/octet-stream";

	string extension = file.substr(dotIndex + 1);

	std::pair<string, string> content_types[] = {
        std::pair<string, string>("html", "text/html"),
        std::pair<string, string>("css", "text/css"),
        std::pair<string, string>("js", "application/javascript"),
        std::pair<string, string>("json", "application/json"),
        std::pair<string, string>("xml", "application/xml"),
        std::pair<string, string>("jpg", "image/jpeg"),
        std::pair<string, string>("jpeg", "image/jpeg"),
        std::pair<string, string>("png", "image/png"),
        std::pair<string, string>("gif", "image/gif"),
        std::pair<string, string>("svg", "image/svg+xml"),
        std::pair<string, string>("ico", "image/x-icon"),
        std::pair<string, string>("pdf", "application/pdf"),
        std::pair<string, string>("zip", "application/zip"),
        std::pair<string, string>("txt", "text/plain"),
        std::pair<string, string>("csv", "text/csv"),
        std::pair<string, string>("mp3", "audio/mpeg"),
        std::pair<string, string>("mp4", "video/mp4"),
        std::pair<string, string>("webm", "video/webm"),
        std::pair<string, string>("ogg", "audio/ogg"),
        std::pair<string, string>("wav", "audio/wav"),
		std::pair<string, string>("", "")};

	for (size_t i = 0; !content_types[i].first.empty(); i++) {
		if (extension == content_types[i].first)
			return content_types[i].first;
	}
	return "application/octet-stream";
}

// returns false if 'file' couldn't be opened
bool Response::fileToBody(string const& file)
{
// std::cout << "FILE: trying to open the file " << file << " .. "; // debug
	std::ifstream ifs(file.c_str());
	if (!ifs.good())
	{
// std::cout << "failed\n"; // debug
		return false;
	}
	std::ostringstream oss;
	oss << ifs.rdbuf();
	_body = oss.str();
// std::cout << "succeeded\n"; // debug

	setField("Content-Type:" + findContentType(file));
	return true;
}

// void Response::confirmationToBody(string const& message, Request const& request)
// {
// 	if (!request.isUser())
// 		return ;
// 	std::ostringstream oss;
// 	oss << "<!DOCTYPE html>\n<html>\n<head>\n<title>upload_confirmation - webserv</title>\n";
// 	oss << "<style>\nhtml{\nbackground-color: #64fc12;\n}\n\n";
// 	oss << "body{\nmin-height: 100vh;\ndisplay: flex;\nalign-items: center;\n";
// 	oss << "justify-content: space-around;\ncolor: #000000;\n}\n</style>\n</head>\n\n";
// 	oss << "<body>\n<center>\n\t<h1 style=\"font-family:'Courier New', Courier, monospace\">";
// 	oss << message;
// 	if (message.empty())
// 		oss << "All done !";
// 	oss << "</h1>\n</center>\n</body>\n</html>\n";
// 	_body = oss.str();
// }

/* MEMBERS */

void	Response::sendResponse(int fd)
{
	addFields();
	string str(wrapPackage());

	if (send(fd, str.c_str(), str.size(), 0) < 1)
		throw SendFailException();
}

string Response::wrapPackage() const
{
	std::ostringstream package;

	package << _version << " " << _status << " " << _reason << "\r\n";
	for (std::map<string, string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
		package << (*it).first << ":" << (*it).second << "\r\n";
	package << "\r\n";
	if (_body.size())
		package << _body;
	return package.str();
}

void Response::addFields()
{
	setHContentLength();
	setHServer();
	if (_status == 405)
		setHAllow();
}

void Response::setHContentLength()
{
	std::ostringstream oss;
	oss << _body.size();
	_fields.erase("Content-Length");
	_fields.insert(std::pair<string, string>("Content-Length", oss.str()));
}

//HTTP format example: "Tue, 15 Nov 1994 08:12:31 GMT"
void Response::setHDate()
{
	std::time_t time = std::time(NULL);
	std::tm* tm = std::gmtime(&time);
	char str[30];
	strftime(str, 29, "%a, %d %b %Y %H:%M:%S GMT", tm);
	str[29] = 0;
	_fields.insert(std::pair<string, string>("Date", str));
}

void Response::setHServer()
{
	_fields.insert(std::pair<string, string>("Server", "WebServ"));
}

void Response::setHAllow()
{
	string value;
	std::map<string, bool>::const_iterator it;

	if (!_location)
		return ;
	for (it = _location->get_limit_except().begin(); it != _location->get_limit_except().end(); ++it)
	{
		if (it->second)
		{
			value = it->first;
			++it;
			break ;
		}
	}
	for (; it != _location->get_limit_except().end(); ++it)
	{
		if (it->second)
			value.append("," + it->first);
	}
	_fields["Allow"] = value;
}

/* DEBUG */

void Response::print(bool do_body) const
{
	std::cout << "response package:\n";
	std::cout << "\t" << _version << " " << _status << " " << _reason << "\n";
	for (std::map<string, string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
		std::cout << "\t" << (*it).first << ":" << (*it).second << "\n";
	if (do_body && _body.size())
		std::cout << "\n\t" << _body << "\n";
}
