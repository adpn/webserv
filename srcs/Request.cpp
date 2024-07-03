#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Entry.hpp"
#include <CGI.hpp>
#include <unistd.h>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>

/* CONSTRUCTORS */

Request::Request(int fd, std::vector<Server *> servers)
	: _status(0), _fin_header(false), _content_left(0), _fd(fd), _is_dir(false), _is_cgi(false), _servers(servers), _server(NULL) {}

Request::Request(Request const& src)
	: _status(src._status), _fin_header(src._fin_header), _content_left(src._content_left), _fd(src._fd),
		_method(src._method), _uri(src._uri), _is_dir(src._is_dir), _is_cgi(src._is_cgi), _version(src._version),
		_body(src._body), _filename(src._filename), _fields(src._fields), _servers(src._servers), _server(src._server) {}

Request::~Request()
	{}

/* G(/S)ETTERS */

bool Request::isValid() const
	{ return (200 <= _status && _status <= 399); }

bool Request::isFin() const
	{ return ((_status && !isValid()) || (_status && _fin_header && _content_left <= 0)); }

bool Request::isUser() const
	{ return _fields.find("Sec-Fetch-User") != _fields.end(); }

// sets _status if bad
bool Request::isGoodSize()
{
	if (!_server->get_request_size() || _body.empty())
		return true;
	if (_fields.find("Content-Length") == _fields.end())
	{
		_status = 411;
		return false;
	}

	if (_content_left > _server->get_request_size())
	{
		_status = 413;
		return false;
	}
	return true;
}

void Request::setCGI()
	{ this->_is_cgi = true; }

int Request::getFd() const
	{ return _fd; }

string const& Request::getMethod() const
	{ return _method; }

int Request::getStatus() const
	{ return _status; }

string const& Request::getUri() const
	{ return _uri; }

string const& Request::getVersion() const
	{ return _version; }

string const& Request::getBody() const
	{ return _body; }

std::map<string, string> const& Request::getFields() const
	{ return _fields; }

/* MEMBERS */

void	Request::assignServer() {
	std::string host = _fields["Host"];

	//Remove the port part of the host
	size_t i = host.find_first_of(":");
	if (i != std::string::npos) {
		host = host.substr(0, i);
	}
	//std::cout << "Host extracted: " << host << std::endl;
	//Loop through servers
	for (size_t i = 0; i < _servers.size(); i++) {
		std::vector<string> names = _servers[i]->get_name();

		//Loop through names of server[i]
		for (size_t j = 0; j < names.size(); j++) {

			if (names[j] == host) {
				_server = _servers[i];
				//std::cout << "Right server found" << std::endl;
				return ;
			}
		}
	}
	//std::cout << "Host not found, default server" << std::endl;
	_server = _servers[0];
}

void Request::defaultErrorPage(Response& response)
{
	std::ostringstream oss;
	oss << "../default_error_pages/" << _status << ".html";
	if (response.fileToBody(oss.str()))
		return ;
	oss.str("");
	oss << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n";
	oss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	oss << "    <link rel=\"icon\" href=\"/favicon.ico\" />\n	<title></title>\n    <!-- <style>\n";
	oss << "        /* Add your CSS styles here */\n    </style> -->\n</head>\n<body>\n    <br><b>ERROR: ";
	oss << _status << " " << response.getReason() << "\n</b></body>\n</html>\n";
	response.setBody(oss.str());
	response.setField("Content-Type: text/html");
}

// returns false if none provided or something else went wrong
bool Request::configErrorPage(Response& response)
{
	std::map<unsigned int, string>::const_iterator it = _server->get_error_page().find(_status);
	if (it == _server->get_error_page().end())
		return false;
	if (!response.fileToBody(it->second))
		return false;
	return true;
}

void Request::handleError(Response& response, int status)
{
	if (!status && !_status)
		_status = 500;
	else if (status)
		_status = status;
	response.setStatus(_status);
	if (!isUser())
		return ;
	if (!configErrorPage(response))
		defaultErrorPage(response);
}

void Request::handleAutoindex(Response &response, Location const* location)
{
	if (access((location->get_root() + _uri).c_str(), F_OK))
		return handleError(response, 404);

	response.setField("Content-Type: text/html");
	std::ostringstream oss;
	oss << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n";
	oss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	oss << "    <link rel=\"icon\" href=\"/favicon.ico\" />\n";
	oss << "<style> body { background-color: #f5f5f5; padding: 20px;} a, h2 {text-align: center; display: block;} ";
	oss << ".centered-box { background-color: #fadcdc;padding: 20px;margin: 0 auto;max-width: 600px; border-radius: 10px;} </style>";
	oss << "<title>Index of " << _uri << "</title>\n  </head>";
	oss << "<body>\n <div class=\"centered-box\">";
	oss << "<h2>Index of " << _uri << "</h2>\n";
	std::vector<Entry> entries = location->create_entries(_uri);

	for (size_t i = 0; i < entries.size(); i++) {
		oss << "<a href=\"./" << entries[i].name;
		if (entries[i].type == DT_DIR)
			oss <<  "/";
		oss <<  "\">" ;
		oss << entries[i].name << "<br>";
	}
	oss << "</div> </body>\n";
	oss << "</html>\n";
	response.setBody(oss.str());
}

void Request::handleReturn(Response &response, Location const* location) const
{
	response.setStatus(location->get_return().first);
	response.setField("Location: " + location->get_return().second);
}

void Request::handleCGI(Response &response, Location const* location)
{
	CGI cgi(response, location, *this);
}

void Request::handleGet(Response& response, Location const* location)
{
	if (location->get_return().first)
		handleReturn(response, location);
	else if (location->get_autoindex() && _is_dir)
		handleAutoindex(response, location);
	else if (!_uri.compare(0, 9, "/cgi-bin/") && _uri.size() > 3 && !_uri.compare(_uri.size() - 3, 3, ".py"))
		handleCGI(response, location);
	else if (!response.fileToBody(getFile(location)))
		handleError(response, 404);
}

void Request::handlePost(Response& response, Location const* location)
{
	if (location->get_upload_path().empty())
		return handleError(response, 403);
	std::ostringstream oss;
	oss << location->get_upload_path() << '/';
	if (_filename.empty())
	{
		std::ostringstream oss_temp;
		for (int i = 0; true; i++)
		{
			oss_temp.str("unnamed");
			oss_temp.seekp(0, std::ios_base::end);
			if (i)
				oss_temp << i;
			if (access((oss.str() + oss_temp.str()).c_str(), W_OK))
				break ;
		}
		oss << oss_temp.str();
	}
	else
		oss << _filename;
	std::ofstream ofs(oss.str().c_str(), std::ios::out | std::ios::trunc);
	if (ofs.fail())
		return handleError(response, 500);
	ofs << _body;
	response.setStatus(204);
}

void Request::handleDelete(Response& response, Location const* location)
{
	if (_is_dir)
		return handleError(response, 403);
	if (std::remove(getFile(location).c_str()))
		return handleError(response, 404);
	response.setStatus(204);
}

void Request::handle()
{
	Response response;
	Location const* location = getLocation();
	response.setStatus(_status);
	response.setLocation(location);

	try
	{
		if (!preHandleChecks(response, location))
		{
			response.sendResponse(_fd);
			return ;
		}

		switch (_method[0])
		{
			case 'G':
				handleGet(response, location);
				break;
			case 'P':
				handlePost(response, location);
				break;
			case 'D':
				handleDelete(response, location);
		}
	}
	catch (std::exception const& e)
	{
		std::cout << "\n*\n* ERROR: request handling: " << e.what() << "\n*\n";
		handleError(response, 500);
	}
	if (!_is_cgi)
		response.sendResponse(_fd);
}

// calls handleError() and returns false if bad
bool Request::preHandleChecks(Response& response, Location const* location)
{
	if (!isValid())
		handleError(response);
	else if (!isGoodSize())
		handleError(response);
	else if (!location)
		handleError(response, 404);
	else if (!location->is_allowed(_method))
		handleError(response, 405);
	else
		return true;
	return false;
}

// returns NULL if location not found
Location const* Request::getLocation()
{
	string search = _uri;
	Location const* ret = find_location(search);
	while (!ret && !search.empty())
	{
		next_search_string(search);
		ret = find_location(search);
	}
	if (ret && _uri.back() == '/')
	{
		struct stat buf;
		if (!stat((ret->get_root() + _uri).c_str(), &buf))
			if (S_ISDIR(buf.st_mode))
				_is_dir = true;
	}
// debug start
// std::cout << "location search for " << _uri << ": found ";
// if (ret)
// {
// std::cout << ret->get_name();
// if (_is_dir)
// std::cout << ", (dir)";
// std::cout << "\n";
// }
// else
// std::cout << "NULL\n";
// debug end
	return ret;
}

Location const* Request::search_locations(string const& search) const
{
	std::list<Location>::const_iterator it;
	for (it = _server->get_locations().begin(); it != _server->get_locations().end(); ++it)
	{
		if (it->get_name() == search)
			return &(*it);
	}
	return NULL;
}

Location const* Request::find_location(string const& search) const
{
	if (search.empty())
		return NULL;
	Location const* ret = search_locations(search);
	if (ret)
		return ret;
	if (search.back() == '/')
		return search_locations(search.substr(0, search.size() - 1));
	else
		return search_locations(search + "/");
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

// no guarantee that return exists/can be opened
string Request::getFile(Location const* location) const
{
	if (_is_dir)
		return getIndexFile(location);
	return location->get_root() + _uri;
}

string Request::getIndexFile(Location const* location) const
{
	string prefix = location->get_root() + _uri;
	if (_uri.back() != '/')
		prefix.append("/");
	size_t i = 0;
	while (i < location->get_index().size())
	{
		if (!access((prefix + location->get_index()[i]).c_str(), R_OK))
			break ;
		++i;
	}
	if (i == location->get_index().size())
		return prefix + "index.html";
	return prefix + location->get_index()[i];
}

// returns true if CRLF was found
bool Request::getline_crlf(std::istringstream& iss, string& buf) const
{
	std::getline(iss, buf, '\r');
	while (iss.good() && iss.peek() != '\n')
	{
		string temp;
		std::getline(iss, temp, '\r');
		buf.append(temp);
	}
	if (!iss.good())
		return false;
	iss.get();
	if (iss.peek() != '\r')
		iss >> std::ws;
	return true;
}

// a 'Host' header is required
void Request::parse(string const& package)
{
	if (_fin_header)
		return parseBody(package);
	if (_status)
		return ;

	std::istringstream iss;
	if (!buffer(package, iss))
		return ;

	if (_method.empty())
		if (!parseRequestLine(iss))
			return ;
	if (!loopFields(iss))
		return ;
	if (!_fin_header)
		return ;
	if (!checkFields())
		return ;
	_status = 200;
	if (!_fin_header)
		return ;
	std::ostringstream oss;
	oss << iss.rdbuf();
	parseBody(oss.str());
	buffer(string(), iss);
	parseBody(iss.str());
}

// returns true and sets iss if a CRLF has been received or _fin_header is set
// if (_fin_header), the remaining static buffer is returned in iss (overwritten) and cleared
bool Request::buffer(string const& package, std::istringstream& iss)
{
	static string buffer;

	buffer.append(package);
	if (_fin_header)
	{
		iss.str(buffer);
		buffer.clear();
		return true;
	}
	size_t pos_crlf = buffer.rfind("\r\n");
	if (pos_crlf == string::npos)
		return false;
	iss.str(buffer.substr(0, pos_crlf + 2));
	buffer.erase(0, pos_crlf + 2);
	return true;
}

bool Request::parseRequestLine(std::istringstream& iss)
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
	return true;
}

bool Request::parseMethod(string const& method)
{
	string	valid_fields[] = {"GET", "POST", "DELETE"};
	int		size = 3;

	for (--size; size >= 0; --size)
		if (!method.compare(valid_fields[size]))
			break ;
	if (size < 0)
	{
		_status = 405;
		return false;
	}
	_method = method;
	return true;
}

// basic parsing, don't allow general "*" (n/a for GET, POST or DELETE)
bool Request::parseUri(string const& uri)
{
	if (uri.empty() || uri[0] != '/')
	{
		_status = 400;
		return false;
	}
	_uri = uri;
	return true;
}

// only allows HTTP
bool Request::parseVersion(string const& version)
{
	if (version != "HTTP/1.1")
	{
		_status = 505;
		return false;
	}
	_version = version;
	return true;
}

// depends on input being CRLF terminated
bool Request::loopFields(std::istringstream& iss)
{
	string token;

	if (!getline_crlf(iss, token))
		return true;
	while (!token.empty())
	{
		if (!parseField(token))
			return false;
		if (!getline_crlf(iss, token))
			return true;
	}
	_fin_header = true;
	return true;
}

bool Request::parseField(string const& header)
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

		manageSpecialField(pair);
		std::pair<std::map<string, string>::iterator, bool> ret(_fields.insert(pair));
		if (!ret.second)
			if (ret.first->second.find(pair.second) == string::npos)
				ret.first->second.append("," + pair.second);
	}
	return true;
}

void Request::parseBody(string const& body)
{
	if (body.empty())
		return ;
	if (_method[0] != 'P' || !isValid())
		return ;
	if (_content_left > _server->get_request_size())
		return ;
	if (isFin())
		return ;
	_body.append(body);
	_content_left -= body.size();
	return ;
}

bool Request::checkFields()
{
	if (_fields.find("Host") == _fields.end())
		_status = 400;
	else
		return true;
	return false;
}

void Request::manageSpecialField(std::pair<string, string> const& pair)
{
	if (_method[0] == 'P' && !pair.first.compare("Content-Length"))
		_content_left = atol(pair.second.c_str());
}

void Request::prepare()
{
	prepareBody();
}

void Request::prepareBody()
{
	if (_method[0] != 'P' || _body.empty())
		return ;
	std::map<string, string>::const_iterator it;
	it = _fields.find("Content-Type");
	if (it != _fields.end() && it->second.find("multipart") != string::npos)
		return removeMultipart(it->second);
}

void Request::removeMultipart(string const& header)
{
	string boundary = extractBoundary(header);
	if (boundary.empty())
		return ;
	extractFilename();
	size_t count = _body.find("\r\n\r\n");
	if (count != string::npos)
		count += 4;
	_body.erase(0, count);
	size_t cursor = _body.find(boundary);
	while (cursor != string::npos && _body.compare(cursor + boundary.size(), 2, "--"))
	{
		count = _body.find("\r\n\r\n", cursor + boundary.size());
		if (count != string::npos)
			count += (4 - cursor);
		_body.erase(cursor, count);
		cursor = _body.find(boundary, cursor);
	}
	_body.erase(cursor);
}

string Request::extractBoundary(string const& header) const
{
	size_t boundary_pos = header.find("boundary=");
	if (boundary_pos == string::npos)
		return string();
	boundary_pos += 9;
	size_t count;
	if (header[boundary_pos] == '"')
	{
		++boundary_pos;
		count = header.find('"', boundary_pos);
	}
	else
		count = header.find_first_of(":;,", boundary_pos);
	if (count != string::npos)
		count -= boundary_pos;
	return "\r\n--" + header.substr(boundary_pos, count);
}

void Request::extractFilename()
{
	size_t name_pos = _body.find("filename=\"");
	if (name_pos == string::npos)
		return ;
	name_pos += 10;
	_filename = _body.substr(name_pos, _body.find('"', name_pos) - name_pos);
}

/* DEBUG */

void Request::print(bool do_body) const
{
	std::cout << "request package:";
	if (!isValid())
		std::cout << " INVALID";
	std::cout << "\n";
	std::cout << "\tstatus: " << _status << "\n";
	if (_method.empty())
		return ;
	std::cout << "\t" << _method << " " << _uri << " " << _version << "\n";
	for (std::map<string, string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
		std::cout << "\t" << (*it).first << ":" << (*it).second << "\n";
	if (do_body && _body.size())
		std::cout << "\n\t" << _body << "\n";
}
