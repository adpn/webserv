#ifndef REQUEST_H
# define REQUEST_H

# include <iostream>
# include <map>

using std::string;

class Request
{
	static string valid_headers[];
	static int	  valid_headers_size;

	char method;		// 'G' or 'P' or 'D'
	string uri;
	string body;
	std::map<string, string> headers; // maybe sumn else i dunno

public:
	Request();
	Request(Request const& src);
	~Request();
	Request& operator=(Request const& rhs);

	char getMethod() const;
	string const& getUri() const;
	string const& getBody() const;
	std::map<string, string> const& getHeaders() const;

	bool parse(string const& package);
};

#endif

/*
		~~HTTP request~~			SP = space , CRLF = carriage return / line feed

Request-Line ;				->	'Method' SP 'Request-URI' SP 'HTTP-Version' CRLF
*(( general-header ;
request-header ;
entity-header ) CRLF) ;
/n
CRLF [ message-body ] ;
 */

 // methods: GET, POST and DELETE
