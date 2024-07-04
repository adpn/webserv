#ifndef CGI_HPP
# define CGI_HPP

# define STDOUT 1
# define TIMEOUT 5

# include <unistd.h>
# include <sys/wait.h>
# include <signal.h>

# include "Request.hpp"
# include "Location.hpp"
# include "Response.hpp"

class CGI {
	private:
		int			_pipe_fd[2];

		Response&		_response;
		Location const*	_location;
		Request&		_request;
		CGI();

		void	_Manager();
		void	_Executor();
		void	_Read();
		void	_Error(unsigned int page_nb);
		void	_Handle();
	public:
		CGI(Response &response, Location const *location, Request &request);

};

#endif
