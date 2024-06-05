#ifndef RESPONSE_H
# define RESPONSE_H

# include <iostream>
# include <map>

using std::string;
class Request;

class Response
{
	string	_version;
	int		_status;
	string	_reason;
	std::map<string, string> _headers;
	string	_body;

	void setContentLength();

public:
	Response();
	Response(Response const& src);
	Response(Request const& request);
	~Response();
	Response& operator=(Response const& rhs);

	bool setStatus(int status);
	void setCustomReason(string const& reason);
	bool setHeader(string const& header);
	void setBody(string const& body);
	bool fileToBody(string const& file);

	ssize_t sendResponse(int fd) const;
	string wrap_package() const;

	// debugging
	void print(bool do_body = true) const;
};

#endif

/*
version SP status-code SP reason-phrase CRLF
headers CRLF
CRLF
body
 */
