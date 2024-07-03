#include "Server.hpp"
#include "Location.hpp"
#include "CGI.hpp"

CGI::CGI(Response &response, Location const *location, Request &request)
	: _response(response), _location(location), _request(request){
		_Handle();
}

void	CGI::_Error(unsigned int page_nb) {
	this->_request.handleError(this->_response, page_nb);
	this->_response.sendResponse(this->_request.getFd());
	exit(1);
}

void	CGI::_Executor(){

	if (dup2(this->_pipe_fd[1], STDOUT) == -1)
		exit(1);
	close(this->_pipe_fd[0]);

	std::string str("/usr/bin/python3");
	std::string file(this->_request.getFile(this->_location));
	file.erase(file.find(".py") + 3);
	if (access(file.c_str(), F_OK | X_OK) < 0)
		exit(2);
	char *cmd_tab[3] = {const_cast<char *>(str.c_str()), const_cast<char *>(file.c_str()), NULL};
	std::string pathInfo("PATH_INFO=" + this->_request.getUri());
	char *envp[] = {const_cast<char *>(pathInfo.c_str()), NULL};

	execve(str.c_str(), cmd_tab, envp);
	close(this->_pipe_fd[1]);
	exit(1);
}

void	CGI::_Read(){
	size_t		bytesRead;
	std::string	output;
	char		buffer[1024];

	while ((bytesRead = read(this->_pipe_fd[0], buffer, sizeof(buffer)))){
		if (bytesRead < 0){
			close(this->_pipe_fd[0]);
			_Error(500);
		}
		buffer[bytesRead] = '\0';
		output += buffer;
	}
	close(this->_pipe_fd[0]);
	this->_response.setStatus(200);
	this->_response.setBody(output);
	this->_response.setField("Content-Type: text/html");
	this->_response.sendResponse(this->_request.getFd());
}

void	CGI::_Manager(){
		int			status;
		std::time_t	beg_time = std::time(NULL);
		pid_t		pid_executor;

		if (pipe(this->_pipe_fd) < 0)
			_Error(500);

		/* executor */
		pid_executor = fork();
		if (pid_executor < 0)
			_Error(500);
		else if (!pid_executor)
			_Executor();

		close(this->_pipe_fd[1]);

		/* timeout */
		while (!waitpid(pid_executor, &status, WNOHANG)){
			if (std::time(nullptr) - beg_time > TIMEOUT){
				kill(pid_executor, SIGKILL);
				close(this->_pipe_fd[0]);
				_Error(508);
			}
		}

		/* script error */
		if (WEXITSTATUS(status)) {
			close(this->_pipe_fd[0]);
			switch (WEXITSTATUS(status)){
				case 2:
					_Error(404);
					break;
				default:
					_Error(500);
			}
		}

		/* send output */
		_Read();
		exit(0);
}

void CGI::_Handle() {
	pid_t		pid_manager;

	this->_request.setCGI();
	/* fork : non_blocking cgi */
	pid_manager = fork();
	if (pid_manager < 0){
		this->_request.handleError(this->_response, 500);
		this->_response.sendResponse(this->_request.getFd());
	}
	else if (!pid_manager)
		_Manager();
}