#include "Response.hpp"
#include "Request.hpp"
#include <sstream>
#include <fstream>
#include <sys/socket.h>

/* CONSTRUCTORS */

Response::Response()
	: version("HTTP/1.1"), status(500), reason("Internal Server Error") {}

Response::Response(Request const& request)
	: version(request.getVersion()), status(500), reason("Internal Server Error") {}

Response::Response(Response const& src)
	{ this->operator=(src); }

Response::~Response()
	{}

Response& Response::operator=(Response const& rhs)
{
	version = rhs.version;
	status = rhs.status;
	reason = rhs.reason;
	headers = rhs.headers;
	body = rhs.body;
	return *this;
}

/* G(/S)ETTERS */

// sets 'reason' if available in internal set
bool Response::setStatus(int status)
{
	this->status = 500;
	this->reason = "Internal Server Error";
	if (status == 500)
		return true;
	if (status < 100 || status > 599)
		return false;
	std::pair<int, string> reasons[] = {
		std::pair<int, string>(100, "Continue"),
		std::pair<int, string>(200, "OK"),
		std::pair<int, string>(201, "Created"),
		std::pair<int, string>(400, "Bad Request"),
		std::pair<int, string>(404, "Not Found"),
		std::pair<int, string>(405, "Method Not Allowed"),
		std::pair<int, string>(413, "Request Entity Too Large"),
		std::pair<int, string>(0, "")};
	for (int i = 0; reasons[i].first; ++i)
	{
		if (reasons[i].first != status)
			continue ;
		this->status = status;
		this->reason = reasons[i].second;
		return true;
	}
	return false;
}

void Response::setCustomReason(string const& reason)
{ this->reason = reason; }

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
		std::pair<std::map<string, string>::iterator, bool> ret(this->headers.insert(pair));
		if (!ret.second)
			if (ret.first->second.find(pair.second) == string::npos)
				ret.first->second.append("," + pair.second);
	}
	return true;
}

void Response::setBody(string const& body)
{
	this->body = body;
	setContentLength();
}

bool Response::fileToBody(string const& file)
{
	std::ifstream ifs(file.c_str());
	if (!ifs.good())
		return false;
	std::ostringstream oss;
	oss << ifs.rdbuf();
	body = oss.str();
	setContentLength();
	return true;
}

void Response::setContentLength()
{
	std::ostringstream oss;
	oss << body.size();
	headers.erase("Content-Length");
	headers.insert(std::pair<string, string>("Content-Length", oss.str()));
}

/* MEMBERS */

// doesn't handle errors :)
ssize_t Response::sendResponse(int fd) const
{
	string str(wrap_package());

	return send(fd, str.c_str(), str.size(), 0);
}

string Response::wrap_package() const
{
	std::ostringstream package;

	package << version << " " << status << " " << reason << "\r\n";
	for (std::map<string, string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		package << (*it).first << ":" << (*it).second << "\r\n";
	package << "\r\n";
	if (body.size())
		package << body;
	return package.str();
}

/* DEBUG */

void Response::print(bool do_body) const
{
	std::cout << "response package:\n";
	std::cout << "\t" << version << " " << status << " " << reason << "\n";
	for (std::map<string, string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		std::cout << "\t" << (*it).first << ":" << (*it).second << "\n";
	if (do_body && body.size())
		std::cout << "\n\t" << body << "\n";
}
