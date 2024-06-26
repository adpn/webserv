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
		int							_status;
		bool						_fin_header;
		long						_content_left;
		int							_fd;
		string						_method;
		string						_uri;
		bool						_is_index;
		string						_version;
		string						_body;
		string						_filename;
		std::map<string, string>	_fields;
		std::vector<Server *>		_servers;
		Server*						_server;

		bool buffer(string const& package, std::istringstream& iss);
		bool parseRequestLine(std::istringstream& iss);
		bool parseMethod(string const& method);
		bool parseUri(string const& uri);
		bool parseVersion(string const& version);
		void parseBody(string const& body);
		bool loopFields(std::istringstream& iss);
		bool parseField(string const& header);
		bool getline_crlf(std::istringstream& iss, string& buf) const;
		bool checkFields();
		void manageSpecialField(std::pair<string, string> const& pair);

		void prepareBody();
		void removeMultipart(string const& header);
		string extractBoundary(string const& header) const;
		void extractFilename();

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

		void		assignServer();

		bool					isValid() const;
		bool					isFin() const;
		bool					isUser() const;
		bool					isGoodSize();
		int						getFd() const;
		string const&			getMethod() const;
		int						getStatus() const;
		string const&			getUri() const;
		string const&			getVersion() const;
		string const&			getBody() const;
		std::map<string, string> const&	getFields() const;

		void			parse(string const& package);
		void			handle();
		void 			prepare();
		Location const*	getLocation();
		string			getFile(Location const* location) const;
		string			getIndexFile(Location const* location) const;

		// debugging
		void print(bool do_body = true) const;
};

#endif
