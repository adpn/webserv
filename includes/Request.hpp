#ifndef REQUEST_H
# define REQUEST_H

# include <iostream>
# include <map>

using std::string;
class Response;

class Request
{
	static std::map<int, Request> requests;

	bool valid;
	bool fin_headers;
	long content_left;
	int fd;
	char method;		// 'G' or 'P' or 'D'
	string uri;
	string version;
	string body;
	std::map<string, string> headers; // merge duplicate headers like so [value1], [value2]
										// exception for 'Set-Cookie' and 'Cookie' headers -> [value1]; [value2]

	// handle errors of these. with (mandatory default) error pages
	bool parseMethod(string const& method);
	bool parseUri(string const& uri);
	bool parseVersion(string const& version);
	bool parseBody(string const& body);
	bool loopHeaders(std::istringstream& iss);
	bool parseHeader(string const& header);
	void getline_crlf(std::istringstream& iss, string& buf) const;
	bool checkHeaders() const;
	void manageSpecialHeader(std::pair<string, string> const& pair);

	void handleGet(Response& response) const;
	void handlePost(Response& response) const;
	void handleDelete(Response& response) const;
	void handleError(Response& response, int status) const;

public:
	Request();
	Request(int fd);
	Request(Request const& src);
	~Request();
	Request& operator=(Request const& rhs);

	static bool loopRequests(int fd, char const* buffer, ssize_t size);
	static Request* loopRequests(int fd);
	static void loopRequests();

	bool isValid() const;
	bool isFin() const;
	int getFd() const;
	char getMethod() const;
	string const& getUri() const;
	string const& getVersion() const;
	string const& getBody() const;
	std::map<string, string> const& getHeaders() const;

	bool parse(string const& package);
	Response handle() const;
	string uritowebsite() const;
	string uritoupload() const;

	// debugging
	void print(bool do_body = true) const;
};

#endif

/*
		~~HTTP request~~			SP = space , CRLF = carriage return / line feed

Request-Line CRLF				->	'Method' SP 'Request-URI' SP 'HTTP-Version' CRLF
*(( general-header ;
request-header ;
entity-header ) CRLF)
CRLF
[ message-body ]
 */

 // methods: GET, POST and DELETE
