// #include <webserv.hpp>
// #include <Server.hpp>
// #include <Client.hpp>

// #include <sys/socket.h>

// void handle_sigint(int sig) {
//     printf("\n[INFO] Fermeture du serveur...\n");
//     close(server_fd);  // Libère le port
//     printf("[INFO] Port libéré, serveur arrêté.\n");
//     exit(0);
// }

#include <vector>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <iostream>
#include <cerrno>
#include <sys/epoll.h>  // epoll_create(), epoll_ctl(), etc. (Linux spécifique)
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <unistd.h>     // POSIX : read(), write(), close()
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{

	// signal(SIGINT, handle_sigint);

	std::string *patate = new std::string;

	for (int i = 0; i < 10; i++) {
		patate->append("Patate\n");
	}
	std::cout << patate << std::endl << *patate << std::endl;
	delete patate;

	return 0;
}