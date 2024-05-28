#ifndef REQUEST_H
# define REQUEST_H

# include <iostream>
# include <map>

using std::string;
class Response;

class Request
{
	bool valid;
	char method;		// 'G' or 'P' or 'D'
	string uri;
	string version;
	string body;
	std::map<string, string> headers; // merge duplicate headers like so [value1], [value2]
										// exception for 'Set-Cookie' and 'Cookie' headers -> [value1]; [value2]

	// handle errors of these. with (mandatory default) error responses
	bool parseMethod(string const& method);
	bool parseUri(string const& uri);
	bool parseVersion(string const& version);
	bool parseBody(string const& body);
	bool parseHeader(string const& header);

public:
	Request();
	Request(Request const& src);
	~Request();
	Request& operator=(Request const& rhs);

	bool isValid() const;
	char getMethod() const;
	string const& getUri() const;
	string const& getVersion() const;
	string const& getBody() const;
	std::map<string, string> const& getHeaders() const;

	bool parse(string const& package);
	Response handle() const;

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
[ message-body ] CRLF (i think it has to end with a CRLF ?)
 */

 // methods: GET, POST and DELETE
