#include "Response.hpp"
#include "Request.hpp"
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <ctime>

/* CONSTRUCTORS */

Response::Response()
	: _version("HTTP/1.1"), _status(500), _reason("Internal Server Error")
	{ setHDate(); }

Response::Response(Request const& request)
	: _version(request.getVersion()), _status(500), _reason("Internal Server Error")
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
	_headers = rhs._headers;
	_body = rhs._body;
	return *this;
}

/* G(/S)ETTERS */

bool Response::isGood() const
	{ return _status < 300; }

string const& Response::getReason() const
	{ return _reason; }

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
		std::pair<int, string>(400, "Bad Request"),
		std::pair<int, string>(403, "Forbidden"),
		std::pair<int, string>(404, "Not Found"),
		std::pair<int, string>(405, "Method Not Allowed"),
		std::pair<int, string>(413, "Request Entity Too Large"),
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

bool Response::setHeader(string const& header)
{
	if (header.find(':') == string::npos)
		return false;
	std::pair<string, string> pair;
	std::istringstream iss(header);

	std::getline(iss, pair.first, ':');
	iss >> std::ws;
	while (!iss.eof())
	{
		std::getline(iss, pair.second, ',');
		iss >> std::ws;
		if (pair.second.empty())
			continue ;
	// do some parsing and comparing to allowed headers here?
		std::pair<std::map<string, string>::iterator, bool> ret(_headers.insert(pair));
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

bool Response::fileToBody(string const& file)
{
// std::cout << "trying to open the file " << file << " .. "; // debug
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
	return true;
}

/* MEMBERS */

// doesn't handle errors :)
ssize_t Response::sendResponse(int fd)
{
	addHeaders();
	string str(wrapPackage());

	return send(fd, str.c_str(), str.size(), 0);
	//not return but throw Response::SendFailException
}

string Response::wrapPackage() const
{
	std::ostringstream package;

	package << _version << " " << _status << " " << _reason << "\r\n";
	for (std::map<string, string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		package << (*it).first << ":" << (*it).second << "\r\n";
	package << "\r\n";
	if (_body.size())
		package << _body;
	return package.str();
}

void Response::addHeaders()
{
	setHContentLength();
	setHServer();
	// maybe content-type if not yet specified i guess
}

void Response::setHContentLength()
{
	std::ostringstream oss;
	oss << _body.size();
	_headers.erase("Content-Length");
	_headers.insert(std::pair<string, string>("Content-Length", oss.str()));
}

//HTTP format example: "Tue, 15 Nov 1994 08:12:31 GMT"
void Response::setHDate()
{
	std::time_t time = std::time(NULL);
	std::tm* tm = std::gmtime(&time);
	char str[30];
	strftime(str, 29, "%a, %d %b %Y %H:%M:%S GMT", tm);
	str[29] = 0;
	_headers.insert(std::pair<string, string>("Date", str));
}

void Response::setHServer()
{
	_headers.insert(std::pair<string, string>("Server", "WebServ"));
}

/* DEBUG */

void Response::print(bool do_body) const
{
	std::cout << "response package:\n";
	std::cout << "\t" << _version << " " << _status << " " << _reason << "\n";
	for (std::map<string, string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		std::cout << "\t" << (*it).first << ":" << (*it).second << "\n";
	if (do_body && _body.size())
		std::cout << "\n\t" << _body << "\n";
}
