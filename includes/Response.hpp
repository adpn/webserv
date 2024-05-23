#ifndef RESPONSE_H
# define RESPONSE_H

# include <iostream>
# include <map>

using std::string;

class Response
{
	string	version;
	int		status;
	string	reason;
	std::map<string, string> headers;
	string	body;

public:
	Response();
	Response(Response const& src);
	~Response();
	Response& operator=(Response const& rhs);

	bool setStatus(int status);
	void setReason(string const& reason);
	bool setHeader(string const& header);
	void setBody(string const& body);

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