#include "Request.hpp"

/* STATICS */
valid_headers = {"GET", "POST", "DELETE"};
valid_headers_size = 3;

/* CONSTRUCTORS */
Request::Request()
	: method(0) {}

Request::Request(Request const& src)
	{ this->operator=(src); }

Request::~Request()
	{}

Request& Request::operator=(Request const& rhs)
{
	method = rhs.method;
	uri = rhs.uri;
	body = rhs.body;
	headers = rhs.headers;
	return *this;
}

/* G(/S)ETTERS */
char Request::getMethod() const
	{ return method; }

string Request::getUri() const
	{ return uri; }

string Request::getBody() const
	{ return body; }

std::map<string, string> Request::getHeaders() const
	{ return headers; }

/* MEMBERS */
bool Request::parse(string const& package)
{
	// parse and fill class
	// e.g.
	int i;
	for (i = 0; i < valid_headers_size; ++i)
		if (!package.compare(0, valid_headers[i].size(), valid_headers[i]))
			break ;
	if (i == valid_headers_size || package[valid_headers[i].size()] != ' ')
		return false;
	// send back an error response here ! (blabla unsupported method blabla)
	// maybe have a class specific error value/string ?
			// -> but we do need to implement default error responses
	method = package[0];

	// ...
}
