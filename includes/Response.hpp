#ifndef RESPONSE_H
# define RESPONSE_H

# include <iostream>
# include <map>
# include <exception>

using std::string;
class Request;
class Location;

class Response
{
		string						_version;
		int							_status;
		string						_reason;
		std::map<string, string>	_fields;
		string						_body;
		Location const*				_location;

		void addFields();
		void setHContentLength();
		void setHDate();
		void setHServer();
		void setHAllow();

	public:
		Response();
		Response(Response const& src);
		Response(Request const& request);
		~Response();
		Response& operator=(Response const& rhs);

		bool			isGood() const;
		string const&	getReason() const;

		void	setLocation(Location const* location);
		bool	setStatus(int status);
		void	setCustomReason(string const& reason);
		bool	setField(string const& field);
		void	setBody(string const& body);
		string	findContentType(string extension);
		bool	fileToBody(string const& file);

		void	sendResponse(int fd);
		string	wrapPackage() const;

		class SendFailException : public std::exception {
		public:
			const char* what() const throw();
		};

		// debug
		void print(bool do_body = true) const;
};

#endif
