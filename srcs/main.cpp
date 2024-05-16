#include <sys/socket.h> //socket
#include <iostream> // cout
#include <netinet/in.h> //struct sockaddr_in
#include <unistd.h> // close

int	main () {

	/*
	AF_INET for IPV4
	SOCK_STREAM for TCP
	0 for default TCP
	*/
	//Create socket
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1) {
		std::cout << "Error" << std::endl;
		return 1;
	}

	int port = 80;
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET; //IPV4 again
	serverAddr.sin_addr.s_addr = INADDR_ANY; //take all IP
	serverAddr.sin_port = htons(port); //which port we will listen to

	//Link socket with settings ?
	if (bind(socketFd, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
		std::cout << "Error" << std::endl;
		close(socketFd);
		return 1;
	}

	#define MAX_CLIENTS 50
	// Listen to socket connections
	if (listen(socketFd, MAX_CLIENTS) == -1) {
		std::cout << "Error" << std::endl;
		close(socketFd);
		return 1;
	}

	while (true) {

		
	}
	return 0;
}