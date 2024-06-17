#ifndef REQUEST_H
# define REQUEST_H

# include <iostream>
# include <map>

using std::string;
class Response;
class Server;
class Location;

class Request
{
		static std::map<int, Request> _requests;

		bool					_valid;
		bool					_fin_headers;
		long					_content_left;
		int						_fd;
		string					_method;
		string					_uri;
		bool					_is_index;
		string					_version;
		string					_body;
		std::map<string, string> _headers;
		Server const&			_server;

		// handle errors of these with (mandatory default) error pages
		bool parseMethod(string const& method);
		bool parseUri(string const& uri);
		bool parseVersion(string const& version);
		bool parseBody(string const& body);
		bool loopHeaders(std::istringstream& iss);
		bool parseHeader(string const& header);
		void getline_crlf(std::istringstream& iss, string& buf) const;
		bool checkHeaders() const;
		void manageSpecialHeader(std::pair<string, string> const& pair);

		void handleGet(Response& response);
		void handlePost(Response& response) const;
		void handleDelete(Response& response) const;
		void handleError(Response& response, int status) const;
		void handleAutoindex(Response& response) const;
		bool preHandleChecks(Response& response) const;

		Location const* find_location(string const& search) const;
		void			next_search_string(string& search) const;

	public:
		Request(int fd, Server const& server);
		Request(Request const& src);
		~Request();

		static bool manageRequests(int fd, Server const& server, char const* buffer, ssize_t size);
		static bool executeRequest(int fd);

		bool isValid() const;
		bool isFin() const;
		bool isGoodSize() const;
		int getFd() const;
		string const& getMethod() const;
		string const& getUri() const;
		string const& getVersion() const;
		string const& getBody() const;
		std::map<string, string> const& getHeaders() const;

		bool parse(string const& package);
		void handle();
		Location const* getLocation();
		string getFile(Location const* location) const;

		// debugging
		void print(bool do_body = true) const;
};

#endif
