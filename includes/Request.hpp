#ifndef REQUEST_H
# define REQUEST_H

# include <iostream>
# include <map>
# include <vector>

using std::string;
class Response;
class Server;
class Location;

class Request
{
		static std::map<int, Request> _requests;

		int							_status;
		bool						_fin_header;
		long						_content_left;
		int							_fd;
		string						_method;
		string						_uri;
		bool						_is_index;
		string						_version;
		string						_body;
		std::map<string, string>	_headers; // fields would have been a more correct name
		std::vector<Server *>		_servers;
		Server*						_server;

		// handle errors of these with (mandatory default) error pages
		bool buffer(string const& package, std::istringstream& iss);
		bool parseRequestLine(std::istringstream& iss);
		bool parseMethod(string const& method);
		bool parseUri(string const& uri);
		bool parseVersion(string const& version);
		void parseBody(string const& body);
		bool loopHeaders(std::istringstream& iss);
		bool parseHeader(string const& header);
		bool getline_crlf(std::istringstream& iss, string& buf) const;
		bool checkHeaders();
		void manageSpecialHeader(std::pair<string, string> const& pair);

		void handleGet(Response& response, Location const* location);
		void handlePost(Response& response, Location const* location);
		void handleDelete(Response& response, Location const* location);
		bool preHandleChecks(Response& response, Location const* location);
		void handleAutoindex(Response& response, Location const* location) const;
		void handleError(Response& response, int status = 0);
		bool configErrorPage(Response& response);
		void defaultErrorPage(Response& response);

		Location const*	find_location(string const& search) const;
		void			next_search_string(string& search) const;

	public:
		Request(int fd, std::vector<Server *> servers);
		Request(Request const& src);
		~Request();

		static bool	manageRequests(int fd, std::vector<Server *> servers, char const* buffer, ssize_t size);
		static bool	executeRequest(int fd);
		void		assignServer();

		bool					isValid() const;
		bool					isFin() const;
		bool					isGoodSize();
		int						getFd() const;
		string const&			getMethod() const;
		int						getStatus() const;
		string const&			getUri() const;
		string const&			getVersion() const;
		string const&			getBody() const;
		std::map<string, string> const&	getHeaders() const;

		void			parse(string const& package);
		void			handle();
		Location const*	getLocation();
		string			getFile(Location const* location) const;
		string			getIndexFile(Location const* location) const;

		// debugging
		void print(bool do_body = true) const;
};

#endif
