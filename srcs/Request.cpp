#include "Request.hpp"
#include "Response.hpp"
#include <sstream>

/* CONSTRUCTORS */

Request::Request()
	: valid(false), method(0) {}

Request::Request(Request const& src)
	{ this->operator=(src); }

Request::~Request()
	{}

Request& Request::operator=(Request const& rhs)
{
	valid = rhs.valid;
	method = rhs.method;
	uri = rhs.uri;
	version = rhs.version;
	body = rhs.body;
	headers = rhs.headers;
	return *this;
}

/* G(/S)ETTERS */

bool Request::isValid() const
	{ return valid; }

char Request::getMethod() const
	{ return method; }

string const& Request::getUri() const
	{ return uri; }

string const& Request::getVersion() const
	{ return version; }

string const& Request::getBody() const
	{ return body; }

std::map<string, string> const& Request::getHeaders() const
	{ return headers; }

/* MEMBERS */

void Request::handleError(Response& response, int status) const
{
	response.setStatus(status);
	// provide appropriate error page?
	// alter response further?
}

void Request::handleGet(Response& response) const
{
	// check if method is allowed on this resource
}

void Request::handlePost(Response& response) const
{
	// check if method is allowed on this resource
}

void Request::handleDelete(Response& response) const
{
	// check if method is allowed on this resource
}

Response Request::handle() const
{
	Response response;

	if (!valid)
		handleError(response, 400);
		return response;
	switch (method)
	{
		case 'G':
			handleGet(response);
			break;
		case 'P':
			handlePost(response);
			break;
		case 'D':
			handleDelete(response);
	}
	return response;
}

// a 'Host' header is required
bool Request::parse(string const& package)
{
	std::istringstream iss(package);
// maybe add more iss bit checks in here?
	string token;
	valid = false;
	std::getline(iss, token, ' ');
	if (!parseMethod(token))
		return false;
	std::cout << "After method" << std::endl;
	std::getline(iss, token, ' ');
	if (!parseUri(token))
		return false;
	std::getline(iss, token, '\r');
	iss >> std::ws;
	if (!parseVersion(token))
		return false;
	std::cout << "After version" << std::endl;
	if (iss.eof())
		return false;
	std::getline(iss, token, '\r');
	iss >> std::ws;
	while (!token.empty())
	{
		if (!parseHeader(token)){
			std::cout << "HERE" << std::endl;
			return false;
		}
		if (iss.eof())
			break ;
		std::getline(iss, token, '\r');
		iss >> std::ws;
	}
	if (!iss.eof())
	{
		std::ostringstream oss;
		oss << iss.rdbuf();
		if (!parseBody(oss.str()))
			return false;
	}
	if (!checkHeaders())
		return false;
	std::cout << "After body" << std::endl;
	valid = true;
	return true;
}

bool Request::parseMethod(string const& method)
{
	string	valid_headers[] = {"GET", "POST", "DELETE"};
	int		size = 3;

	if (method.size() < 3)
		return false;
	for (--size; size >= 0; --size)
		if (!method.compare(valid_headers[size]))
			break ;
	if (size < 0)
		return false;
	this->method = method[0];
	return true;
}

// basic parsing, don't allow general "*" (n/a for GET, POST or DELETE)
bool Request::parseUri(string const& uri)
{
	if (uri.empty())
		return false;
	if (uri.compare(0, 7, "http://") && uri[0] != '/')
		return false;
	this->uri = uri;
	return true;
}

// only allows HTTP
bool Request::parseVersion(string const& version)
{
	if (version.compare(0, 5, "HTTP/"))
		return false;
	this->version = version;
	return true;
}

bool Request::parseHeader(string const& header)
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

		std::pair<std::map<string, string>::iterator, bool> ret(this->headers.insert(pair));
		if (!ret.second)
			if (ret.first->second.find(pair.second) == string::npos)
				ret.first->second.append("," + pair.second);
	}
	return true;
}

// doesn't check anything
bool Request::parseBody(string const& body)
{
	this->body = body;
	return true;
}

bool Request::checkHeaders() const
{
	if (headers.find("Host") == headers.end())
		return false;
	return true;
}

/* DEBUG */

void Request::print(bool do_body) const
{
	std::cout << "request package:";
	if (!valid)
		std::cout << " INVALID";
	std::cout << "\n";
	if (!method)
		return ;
	std::cout << "\t" << method << " " << uri << " " << version << "\n";
	for (std::map<string, string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		std::cout << "\t" << (*it).first << ":" << (*it).second << "\n";
	if (do_body && body.size())
		std::cout << "\n\t" << body << "\n";
}
