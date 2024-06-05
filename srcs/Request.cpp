#include "Request.hpp"
#include "Response.hpp"
#include <sstream>

/* CONSTRUCTORS */

Request::Request()
	: valid(false), fin_headers(false), content_left(0), method(0) {}

Request::Request(int fd)
	: valid(false), fin_headers(false), content_left(0), fd(fd), method(0) {}

Request::Request(Request const& src)
	{ this->operator=(src); }

Request::~Request()
	{}

Request& Request::operator=(Request const& rhs)
{
	valid = rhs.valid;
	fin_headers = rhs.fin_headers;
	content_left = rhs.content_left;
	fd = rhs.fd;
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

bool Request::isFin() const
	{ return (fin_headers && content_left <= 0); }

int Request::getFd() const
	{ return fd; }

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

/* STATIC MEMBERS */

std::map<int, Request> Request::requests;

// returns true if all requests are finished (call with fd == 0 to check)
// handles finished requests and sends back a response
bool Request::loopRequests(int fd, char const* buffer, ssize_t size)
{
	if (!fd)
		return requests.empty();
	std::string package(buffer, buffer + size);
	std::map<int, Request>::iterator it = (requests.insert(std::pair<int, Request>(fd, Request(fd)))).first;
	Request& instance = (*it).second;
	instance.parse(package);
//debug
// std::cout << ">> parsed a packet[" << fd << "], " << instance.content_left << "b left\n";
	// if (!instance.isFin())
	// 	return false;
//debug
// std::cout << ">> "; instance.print();
	// instance.handle();
	// requests.erase(it);
	return requests.empty();
}

Request* Request::loopRequests(int fd)
{
	std::map<int, Request>::iterator it = requests.find(fd);
	if (!(*it).first)
		return NULL;
	return &(*it).second;
}

// clears the map
void Request::loopRequests()
{
	requests.clear();
}

/* MEMBERS */

void Request::handleError(Response& response, int status) const
{
	response.setStatus(status);
	// provide appropriate error page?
	std::ostringstream oss;
	oss << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n";
	oss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	oss << "    <link rel=\"icon\" href=\"./favicon.ico\" />\n	<title></title>\n    <!-- <style>\n";
	oss << "        /* Add your CSS styles here */\n    </style> -->\n</head>\n<body>\n    <br><b>ERROR: ";
	oss << status << " " << response.getReason() << "\n</b></body>\n</html>\n";
	response.setBody(oss.str());
	// alter response further?
}

void Request::handleGet(Response& response) const
{
	response.setStatus(200);
	response.setHeader("Content-Type: text/html");
	// check below if method is allowed on this resource
	if (!true)
		handleError(response, 405);
	else if (!response.fileToBody(uritowebsite()))
		handleError(response, 404);
}

void Request::handlePost(Response& response) const
{
	(void)response;
	// check if method is allowed on this resource
	// temp error
	handleError(response, 405);

	/* multipart;
		header: Content-Type: multipart/form-data;boundary="boundary"
		body:	--boundary
				Content-Disposition: form-data; name="field1"

				value1
				--boundary
				Content-Disposition: form-data; name="field2"; filename="example.txt"

				value2
				--boundary--
	*/
}

void Request::handleDelete(Response& response) const
{
	(void)response;
	// check if method is allowed on this resource
	// temp error
	handleError(response, 405);
}

Response Request::handle() const
{
	Response response;
	char expr = 0;
	if (valid)
		expr = method;
	switch (expr)
	{
		case 'G':
			handleGet(response);
			break;
		case 'P':
			handlePost(response);
			break;
		case 'D':
			handleDelete(response);
			break;
		default:
			handleError(response, 400);
	}
	return response;
}

string Request::uritowebsite() const
{
	if (!uri.compare("/"))
		return "website/index.html";
	return string("website") + uri;
}

string Request::uritoupload() const
{
	// translate uri to a filepath
	return string();
}

void Request::getline_crlf(std::istringstream& iss, string& buf) const
{
	std::getline(iss, buf, '\r');
	while (iss.peek() != '\n')
	{
		string temp;
		std::getline(iss, temp, '\r');
		buf.append(temp);
	}
	iss.get();
	if (iss.peek() == '\r')
		return ;
	iss >> std::ws;
}

// a 'Host' header is required
bool Request::parse(string const& package)
{
	if (fin_headers && content_left)
		return parseBody(package);

// maybe add more iss bit checks in here?
	std::istringstream iss(package);
	if (!method)
	{
		string token;
		std::getline(iss, token, ' ');
		if (!parseMethod(token))
			return false;
		std::getline(iss, token, ' ');
		if (!parseUri(token))
			return false;
		getline_crlf(iss, token);
		if (!parseVersion(token))
			return false;
		if (iss.eof())
			return false;
			// what is this check ?
	}
	if (!fin_headers)
	{
		if (!loopHeaders(iss))
			return false;
		if (!fin_headers)
			return true;
		if (!checkHeaders())
			return false;
		valid = true;
	}
	std::ostringstream oss;
	oss << iss.rdbuf();
	if (!parseBody(oss.str()))
		return false;
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

bool Request::loopHeaders(std::istringstream& iss)
{
	string token;
	static string leftover;

	if (leftover.empty())
		getline_crlf(iss, token);
	else
	{
		token = leftover;
		getline_crlf(iss, leftover);
		token.append(leftover);
	}
	while (!token.empty())
	{
		if (iss.eof())
		{
			leftover = token;
			return true;
		}
		if (!parseHeader(token)){
			return false;
		}
		getline_crlf(iss, token);
	}
	leftover.clear();
	fin_headers = true;
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

		manageSpecialHeader(pair);
		std::pair<std::map<string, string>::iterator, bool> ret(this->headers.insert(pair));
		if (!ret.second)
			if (ret.first->second.find(pair.second) == string::npos)
				ret.first->second.append("," + pair.second);
	}
	return true;
}

// doesn't really check anything
bool Request::parseBody(string const& body)
{
// check if a request is finished with content-length
// (or an empty chunk at the end IF it is chunked (chunked in Transfer-Encoding header))
	if (isFin())
		return false;
	this->body.append(body);
	content_left -= body.size();
	return true;
}

bool Request::checkHeaders() const
{
	if (headers.find("Host") == headers.end())
		return false;
	return true;
}

void Request::manageSpecialHeader(std::pair<string, string> const& pair)
{
	// if (!pair.first.compare("Content-Type"))
	// {
	// 	if (pair.second.find("multipart") != string::npos)
	// 		boundary = pair.second.substr(pair.second.find("boundary") + 9, string::npos);
	// 		// maybe replace the string::npos with a check for delimiters?
	// }
	if (!pair.first.compare("Content-Length"))
		content_left = atol(pair.second.c_str());
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
