#include "Response.hpp"
#include <sstream>


/* CONSTRUCTORS */
Response::Response()
	: version("HTTP/1.1"), status(500) {}

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
bool Response::setStatus(int status)
{
	if (status < 100 || status > 599)
		return false;
	this->status = status;
	return true;
}

void Response::setReason(string const& reason)
{
	// maybe some kind of check?
	this->reason = reason;
}

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
}

/* MEMBERS */
string Response::wrap_package() const
{
	std::ostringstream package;

	package << version << " " << status << " " << reason << "\r\n";
	for (std::map<string, string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		package << (*it).first << ":" << (*it).second << "\r\n";
	}
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
