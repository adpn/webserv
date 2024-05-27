#include <iostream> // cout
#include <Socket.hpp>
#include <vector>
#include <sys/poll.h> //pollfd

#define BUFFER_SIZE 1024

int main() {

	std::vector<Socket> serverSockets;
	std::vector<pollfd> fds;
	
	serverSockets.reserve(2); //number of sockets
	std::cout << "Capacity: " << serverSockets.capacity() << std::endl;
	Socket sock1(8080);
	Socket sock2(8081);
	serverSockets.push_back(sock1);
	serverSockets.push_back(sock2);


	std::cout << "For loop start:" << std::endl;
	for (size_t i = 0; i < serverSockets.size(); i++) {
		struct pollfd pfd;
		pfd.fd = serverSockets[i]; // first elements are for servers sockets
		std::cout << "Here" << std::endl;
		pfd.events = POLLIN;	   // server sockets is just to check events, real communication is done with clients sockets
		fds.push_back(pfd);
	}

	std::cout << "Main loop start:" << std::endl;
	while (true)
	{
		// Last argument is timeout time
		if (poll(&fds[0], fds.size(), 10000) == -1) {
			std::cout << "Error poll";
			return 1;
		}

		/*
		fds = array of all fd
		go through all the fds to catch received events, especially on servers fds to detect new connection
		*/
		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].revents & POLLIN) {
				//case where the POLLIN is on a server fd
				if (i < serverSockets.size()) {
					// Accept new client connection
					struct sockaddr_in clientAddr;
					socklen_t clientAddrLen = sizeof(clientAddr);
					int clientFd = accept(serverSockets[i], (struct sockaddr *)&clientAddr, &clientAddrLen);
					if (clientFd == -1) {
						std::cout << "Accept error" << std::endl;
						continue;
					}
					std::cout << "Client connected to port " << serverSockets[i].getPort() << std::endl;

					// Add new client socket to fds
					struct pollfd pfd;
					pfd.fd = clientFd;
					pfd.events = POLLIN | POLLOUT;
					fds.push_back(pfd);
				}
				else {
				//case where the POLLIN is on a client fd
					char buffer[BUFFER_SIZE];
                    ssize_t bytesReceived = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytesReceived > 0) {
                        buffer[bytesReceived] = '\0';
                        std::cout << "Received from client: " << buffer << std::endl;
						
						//manage client

						close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                    } else if (bytesReceived == 0) {
                        // Connection closed by client
                        std::cout << "Client disconnected" << std::endl;

                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                    } else {
                        std::cout << "Recv error" << std::endl;
                        return 1;
                    }
				}
			}
			else if (fds[i].revents & POLLOUT) {
				std::cout << "Ready to send data to client on fd " << fds[i].fd << std::endl;
				// Send data to client
				const char *msg = "Hello from server";
				send(fds[i].fd, msg, strlen(msg), 0); //need to protect it
			}
		}
	}
	return 0;
}
