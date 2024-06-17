#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Entry.hpp"
#include <sstream>
#include <dirent.h>

/* CONSTRUCTORS */

Request::Request(int fd, Server const& server)
	: _valid(false), _fin_headers(false), _content_left(0), _fd(fd), _is_index(false), _server(server) {}

Request::Request(Request const& src)
	: _valid(src._valid), _fin_headers(src._fin_headers), _content_left(src._content_left),
		_fd(src._fd), _method(src._method), _uri(src._uri), _is_index(src._is_index),
		_version(src._version), _body(src._body), _headers(src._headers), _server(src._server) {}

Request::~Request()
	{}

/* G(/S)ETTERS */

bool Request::isValid() const
	{ return _valid; }

bool Request::isFin() const
	{ return (_fin_headers && _content_left <= 0); }

int Request::getFd() const
	{ return _fd; }

string const& Request::getMethod() const
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

std::map<int, Request> Request::_requests;

// returns true if request is finished
// return false if request is not finished or fd is bad
bool Request::manageRequests(int fd, Server const& server, char const* buffer, ssize_t size)
{
	if (fd < 3)
		return false;
	std::string package(buffer, buffer + size);
	std::map<int, Request>::iterator it = (_requests.insert(std::pair<int, Request>(fd, Request(fd, server)))).first;
	Request& instance = (*it).second;
	instance.parse(package);
//debug
//std::cout << ">> parsed a packet[" << fd << "], " << instance.content_left << "b left\n";
	if (!instance.isFin())
		return false;
//debug
//std::cout << ">> finished a packet[" << fd << "]:\n"; instance.print();
	return true;
}

// returns true if execution was a success
// else fd was not found or request is unfinished
bool Request::executeRequest(int fd)
{
	std::map<int, Request>::iterator it = _requests.find(fd);
	if (it == _requests.end())
		return false;
	Request& instance = (*it).second;
	if (!instance.isFin())
		return false;
	instance.handle();
	_requests.erase(it);
	return true;
}

/* MEMBERS */

void Request::handleError(Response& response, int status) const
{
	response.setStatus(status);
	// provide appropriate error page?
	std::ostringstream oss;
	oss << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n";
	oss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	oss << "    <link rel=\"icon\" href=\"/favicon.ico\" />\n	<title></title>\n    <!-- <style>\n";
	oss << "        /* Add your CSS styles here */\n    </style> -->\n</head>\n<body>\n    <br><b>ERROR: ";
	oss << status << " " << response.getReason() << "\n</b></body>\n</html>\n";
	response.setBody(oss.str());
	// alter response further?
}

void Request::handleGet(Response& response)
{
	// use try catch maybe?
	Location const* location;
	string file;

	response.setStatus(200);
	response.setHeader("Content-Type: text/html"); //if .css should be text/css

	location = getLocation();
	if (!location)
		handleError(response, 404);
	else if (!(*location).is_allowed(_method))
		handleError(response, 405);
	else if ((*location).get_autoindex())
		handleAutoindex(response);
	else if (!response.fileToBody(getFile(location)))
		handleError(response, 404);
}

void Request::handleAutoindex(Response &response) const {
	std::ostringstream oss;
	oss << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n";
	oss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	oss << "    <link rel=\"icon\" href=\"/favicon.ico\" />\n";
	// oss << "<link rel=\"stylesheet\" href=\"coucou.css\">";
	oss << "<title>Index of " << _uri << "</title>\n  </head>";
	oss << "<body>\n";
	oss << "<h2>Index of " << _uri << "</h2>\n";
	std::vector<Entry> entries;
	entries.push_back(Entry("images", DT_DIR));
	entries.push_back(Entry("fuckbert", DT_DIR));
	entries.push_back(Entry("helloDenis.html", 0));

	for (size_t i = 0; i < entries.size(); i++) {
		oss << "<a href=\"./" << entries[i].name;
		if (entries[i].type == DT_DIR)
			oss <<  "/";
		oss <<  "\">" ;
		oss << entries[i].name << "<br>";
	}
	oss << "</body>\n";
	oss << "</html>\n";
	response.setBody(oss.str());
}

void Request::handlePost(Response& response) const
{
	(void)response;
	if (/* Method not in location */!true)
		handleError(response, 405);
// temp error
//handleError(response, 405);
	// check if method is allowed on this resource (405)
	// check if resource already exists, if yes, overwrite (200)
	// if not, create new resource (201)

	/* multipart;	or... keep it intact and send it back the same way ? (with correct headers)
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
	if (/* Method not in location */!true)
		handleError(response, 405);
// temp error
handleError(response, 405);
	// we should probably be VERY careful with deleting stuff ... probably or not :)
	// check if resource exists? (404)
	// check if resource is part of server (403 or 404 if we're not doing the last step)
	// check if method is allowed on this resource (405)
	// if (!std::remove(uritoupload().c_str()))
		// response.setStatus(204);
}

void Request::handle()
{
	Response response;
	char expr = 0;
	if (_valid)
		expr = _method[0];
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
	response.sendResponse(_fd);
}

// returns NULL if location not found
Location const* Request::getLocation()
{
	// special uri's ?
	string search = _uri;
	Location const* ret = find_location(search);
	if (ret)
		_is_index = true;
	while (!ret && !search.empty())
	{
		next_search_string(search);
		ret = find_location(search);
	}
// debug
// std::cout << "location search for " << _uri << ": found ";
// if (ret)
// {
// std::cout << ret->get_name();
// if (_is_index)
// std::cout << " (index)";
// std::cout << "\n";
// }
// else
// std::cout << "NULL\n";
// debug end
	return ret;
}

Location const* Request::find_location(string const& search) const
{
	// redirect if location == uri + / (to uri + /) (ofc only if there is no location == uri)
	if (search.empty())
		return NULL;
	std::map<string, Location&>::const_iterator it;
	it = _server.get_aliases().find(search);
	if (it == _server.get_aliases().end())
	{
		if (search.back() == '/')
			it = _server.get_aliases().find(search.substr(0, search.size() - 1));
		else
			it = _server.get_aliases().find(search + "/");
		if (it == _server.get_aliases().end())
			return NULL;
	}
	return &it->second;
}

void Request::next_search_string(string& search) const
{
	if (search.empty())
		return ;
	size_t slash_pos = search.size() - 1;
	while (slash_pos && search[slash_pos] == '/')
		--slash_pos;
	slash_pos = search.rfind('/', slash_pos);
	if (slash_pos == string::npos)
	{
		search = '/';
		return ;
	}
	search.erase(slash_pos + 1);
}

// returns empty string if file not found
string Request::getFile(Location const* location) const
{
	if (_is_index)
	{
		// return default (or autoindex?)
		// what if no default and no autoindex ?
		// loop over get_index until one works

		if (location->get_root() == "/")
			return location->get_index()[0];
		return location->get_root() + "/" + location->get_index()[0];
	}

	if (location->get_root() == "/")
		return _uri.substr(1);
	return location->get_root() + _uri;
}

/* alternative to try for ^ !

	string file(location->get_root());
	if (_is_index)
	{
		// return default (or autoindex?)
		// what if no default and no autoindex ?
		// loop over get_index until one works

		file.append("/" + location->get_index()[0]);
	}
	else
	{
		file.append(_uri);
	}
	file.erase(0, file.find_first_not_of('/'));
	file.erase(file.find_last_not_of('/') + 1);
	return file;
*/

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
	if (_fin_headers && _content_left)
		return parseBody(package);

// maybe add more iss bit checks in here?
	std::istringstream iss(package);
	if (_method.empty())
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
	_method = method;
	return true;
}

// basic parsing, don't allow general "*" (n/a for GET, POST or DELETE)
bool Request::parseUri(string const& uri)
{
	if (uri.empty())
		return false;
	if (uri.compare(0, 7, "http://") && uri[0] != '/') //wtf is that supposed to do ???
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
	if (_method[0] != 'P')
		return false;
// check if a request is finished with content-length
// (or an empty chunk at the end IF it is chunked (chunked in Transfer-Encoding header))
	if (isFin())
		return false;
	_body.append(body);
	_content_left -= body.size();
	return true;
// complain if method is post but there's no Content-Length? (411 Length Required)
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
	if (_method.empty())
		return ;
	std::cout << "\t" << _method << " " << _uri << " " << _version << "\n";
	for (std::map<string, string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		std::cout << "\t" << (*it).first << ":" << (*it).second << "\n";
	if (do_body && _body.size())
		std::cout << "\n\t" << _body << "\n";
}
