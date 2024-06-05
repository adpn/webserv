#include "Request.hpp"
#include "Response.hpp"
#include <sstream>

/* CONSTRUCTORS */

Request::Request()
	: _valid(false), _fin_headers(false), _content_left(0), _method(0) {}

Request::Request(int fd)
	: _valid(false), _fin_headers(false), _content_left(0), _fd(fd), _method(0) {}

Request::Request(Request const& src)
	{ this->operator=(src); }

Request::~Request()
	{}

Request& Request::operator=(Request const& rhs)
{
	_valid = rhs._valid;
	_fin_headers = rhs._fin_headers;
	_content_left = rhs._content_left;
	_fd = rhs._fd;
	_method = rhs._method;
	_uri = rhs._uri;
	_version = rhs._version;
	_body = rhs._body;
	_headers = rhs._headers;
	return *this;
}

/* G(/S)ETTERS */

bool Request::isValid() const
	{ return _valid; }

bool Request::isFin() const
	{ return (_fin_headers && _content_left <= 0); }

int Request::getFd() const
	{ return _fd; }

void Request::setFd(int fd)
	{ _fd = fd; }

char Request::getMethod() const
	{ return _method; }

string const& Request::getUri() const
	{ return _uri; }

string const& Request::getVersion() const
	{ return _version; }

string const& Request::getBody() const
	{ return _body; }

std::map<string, string> const& Request::getHeaders() const
	{ return _headers; }

/* STATIC MEMBERS */

// returns true if request added or executed correctly
// return false if request doesn't exist, or is not finished
bool Request::manageRequests(int fd, string const& package, int execute)
{
	static std::map<int, Request> requests;	

	if (!fd)
		return false;
	if (!execute) {
		std::map<int, Request>::iterator it = (requests.insert(std::pair<int, Request>(fd, Request()))).first;
		Request& instance = (*it).second;
		instance.parse(package, fd);
		//debug
		//std::cout << ">> parsed a packet[" << fd << "], " << instance.content_left << "b left\n";
		if (!instance.isFin())
			return false;
		return true;
	}
	std::map<int, Request>::iterator it = requests.find(fd);
	if (it == requests.end())
		return false;
	Request& instance = (*it).second; 
	//debug
	//std::cout << ">> "; instance.print();
	if (!instance.isFin())
			return false;
	instance.handle();
	requests.erase(it);
	return true;
}

/* MEMBERS */

void Request::handleError(Response& response, int status) const
{
	response.setStatus(status);
	// provide appropriate error page?
	// alter response further?
}

void Request::handleGet(Response& response) const
{
	//(void)response;
	// std::cout << "HERE" << std::endl;
	std::string httpHeader = "Content-Type: text/html\r\n";
	response.setStatus(200);
	response.setHeader(httpHeader);
	response.fileToBody("website/index.html");
	//if not allowed
	//
	// check if method is allowed on this resource
}

void Request::handlePost(Response& response) const
{
	(void)response;
	// check if method is allowed on this resource
}

void Request::handleDelete(Response& response) const
{
	(void)response;
	// check if method is allowed on this resource
}

void Request::handle() const
{
	Response response;
	char todo = 0;

	if (_valid)
		todo = _method;
	switch (todo)
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
	response.sendResponse(_fd);
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
//maybe take an fd as input also to not use setFd
bool Request::parse(string const& package, int fd)
{
	_fd = fd;
	if (_fin_headers && _content_left)
		return parseBody(package);

// maybe add more iss bit checks in here?
	std::istringstream iss(package);
	if (!_method)
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
	if (!_fin_headers)
	{
		if (!loopHeaders(iss))
			return false;
		if (!_fin_headers)
			return true;
		if (!checkHeaders())
			return false;
		_valid = true;
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
	_method = method[0];
	return true;
}

// basic parsing, don't allow general "*" (n/a for GET, POST or DELETE)
bool Request::parseUri(string const& uri)
{
	if (uri.empty())
		return false;
	if (uri.compare(0, 7, "http://") && uri[0] != '/')
		return false;
	_uri = uri;
	return true;
}

// only allows HTTP
bool Request::parseVersion(string const& version)
{
	if (version.compare(0, 5, "HTTP/"))
		return false;
	_version = version;
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
	_fin_headers = true;
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
		std::pair<std::map<string, string>::iterator, bool> ret(_headers.insert(pair));
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
	_body.append(body);
	_content_left -= body.size();
	return true;
}

bool Request::checkHeaders() const
{
	if (_headers.find("Host") == _headers.end())
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
		_content_left = atol(pair.second.c_str());
}

/* DEBUG */

void Request::print(bool do_body) const
{
	std::cout << "request package:";
	if (!_valid)
		std::cout << " INVALID";
	std::cout << "\n";
	if (!_method)
		return ;
	std::cout << "\t" << _method << " " << _uri << " " << _version << "\n";
	for (std::map<string, string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		std::cout << "\t" << (*it).first << ":" << (*it).second << "\n";
	if (do_body && _body.size())
		std::cout << "\n\t" << _body << "\n";
}
