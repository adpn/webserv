#include <iostream> // cout
#include <vector>
#include <cstring>
#include <sys/poll.h> //pollfd
#include <Socket.hpp>
#include <Request.hpp>
#include <Response.hpp>

//buffer size should be defined with client_max_body_size ????????
#define BUFFER_SIZE 1024

int managePollin(std::vector<pollfd>& fds, std::vector<Socket>& serverSockets, size_t i)
{
	// case where the POLLIN is on a server fd
	if (i < serverSockets.size()) {
		std::cout << "POLLIN SERVER" << std::endl;
		// Accept new client connection
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		int clientFd = accept(serverSockets[i], (struct sockaddr *)&clientAddr, &clientAddrLen);
		if (clientFd == -1) {
			std::cout << "Accept error" << std::endl;
			return 0;
		}
		std::cout << "Client connected to port " << serverSockets[i].getPort() << std::endl;

		// Add new client socket to fds
		struct pollfd pfd;
		pfd.fd = clientFd;
		pfd.events = POLLIN; // Initially only enable POLLIN
		fds.push_back(pfd);
	}
	else {
		std::cout << "POLLIN CLIENT: " << fds[i].fd << std::endl;
		// case where the POLLIN is on a client fd
		char buffer[BUFFER_SIZE];
		ssize_t bytesReceived = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			Request request;
			std::cout << "Received from client: " << std::endl;
			std::cout << buffer << std::endl;
			Request::loopRequests(fds[i].fd, buffer, bytesReceived);
			// if (!request.parse(buffer)) [DEPRECATED]
			// 	exit(1); [DEPRECATED]
			// request.print();
			// manage client
			fds[i].events |= POLLOUT; // Enable POLLOUT to send data
			// close(fds[i].fd);
			// fds.erase(fds.begin() + i);
			// --i;
		}
		else if (bytesReceived == 0) {
			// Connection closed by client
			std::cout << "Client disconnected " << fds[i].fd <<std::endl;

			close(fds[i].fd);
			fds.erase(fds.begin() + i);
			--i;
		}
		else {
			std::cout << "Recv error" << std::endl;
			return 1;
		}
	}
	return 0;
}

int managePollout(std::vector<pollfd>& fds, size_t i) {
	std::cout << "Ready to send data to client on fd " << fds[i].fd << std::endl;
	// Send data to client
	// const char *msg = "Hello from server";
	const char *htmlPage =
		"<!DOCTYPE html>"
		"<html lang=\"en\">"
		"<head>"
		"<meta charset=\"UTF-8\">"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
		"<title>Hello Bert</title>"
		"<link rel=\"icon\" href=\"./favicon.ico\"/>"
		"<!-- <style>"
		"</style> -->"
		"</head>"
		"<body>"
		"<header>"
		"<h1>Hello Bert</h1>"
		"</header>"
		"<nav>"
		"<!-- Navigation menu can go here -->"
		"</nav>"
		"<main>"
		"<section>"
		"<!-- <h2>Section 1</h2> -->"
		"<p>You are not commiting.</p>"
		"</section>"
		"<section>"
		"<!-- <h2>Section 2</h2> -->"
		"<p>You should commit more.</p>"
		"</section>"
		"<h2>Upload a File</h2>"
   		"<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
        "<label for=\"file\">Choose a file:</label>"
        "<input type=\"file\" id=\"file\" name=\"file\">"
        "<br><br>"
        "<input type=\"submit\" value=\"Upload\">"
    	"</form>"
		"</main>"
		"<footer>"
		"<p>&copy; I don't care</p>"
		"</footer>"
		"</body>"
		"</html>";

	std::string httpHeader =
		"Content-Type: text/html\r\n"
		"Content-Length: " + std::to_string(std::strlen(htmlPage)) + "\r\n";
	Response response;
	response.setStatus(200);
	response.setHeader(httpHeader);
	response.setBody(htmlPage);

	std::string httpResponse = response.wrap_package();
	ssize_t bytesSent = send(fds[i].fd, httpResponse.c_str(), httpResponse.size(), 0);
	if (bytesSent < 0) {
		std::cout << "Send error" << std::endl;
		return 1; //should I stop the program if send fails ????
	}
	// After sending data, disable POLLOUT until more data needs to be sent
	fds[i].events = POLLIN;
	return 0;
}

int main()
{

	std::vector<Socket> serverSockets;
	std::vector<pollfd> fds;

	serverSockets.reserve(2); // number of sockets
	std::cout << "Capacity: " << serverSockets.capacity() << std::endl;
	Socket sock1(8080);
	Socket sock2(8081);
	serverSockets.push_back(sock1);
	serverSockets.push_back(sock2);

	std::cout << "For loop start:" << std::endl;
	for (size_t i = 0; i < serverSockets.size(); i++)
	{
		struct pollfd pfd;
		pfd.fd = serverSockets[i]; // first elements are for servers sockets
		pfd.events = POLLIN; // server sockets is just to check events, real communication is done with clients sockets
		fds.push_back(pfd);
	}

	std::cout << "Main loop start:" << std::endl;
	while (true)
	{
		// Last argument is timeout time
		if (poll(&fds[0], fds.size(), 10000) == -1)
		{
			std::cout << "Error poll";
			return 1;
		}

		/*
		fds = array of all fd
		go through all the fds to catch received events, especially on servers fds to detect new connection
		*/
		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN) {
				if (managePollin(fds, serverSockets, i))
					return 1;
			}
			else if (fds[i].revents & POLLOUT) {
				if (managePollout(fds, i))
					return 1;
			}
		}
	}
	return 0;
}
