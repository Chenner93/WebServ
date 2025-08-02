#include <webserv.hpp>
#include <Server.hpp>
#include <Client.hpp>

#include <sys/socket.h>

bool	g_runWebserv = true;

void	closeWebserv(int sig) {
	g_runWebserv = false;
	std::cout << std::endl << RED"[INFO] Shutting Down Server(s)..."RESET << std::endl;
}

void	tmp_config(int ac, std::vector<Server> &server) {
	if (ac != 2)
		std::cerr << "_.conf file only, but we continue because it's just a test" << std::endl;

	Server	s1, s2, s3;

	s1.setServer("Alpha", "127.0.0.1", 8081);
	s2.setServer("Beta", "127.0.0.1", 8082);
	s3.setServer("Gamma", "127.0.0.1", 8083);

	server.reserve(3);

	server.push_back(s1);
	server.push_back(s2);
	server.push_back(s3);

	// return server;
}

int main(int ac, char **av) {

	signal(SIGINT, closeWebserv);

	//check arguments and parse config_file, return vector of Server
	std::vector<Server>	servers;
	tmp_config(ac, servers);

	int	epoll_fd = epoll_create1(0);

	if (epoll_fd == -1) {
		// std::cerr << RED"Error: "RESET << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	int	new_socket;
	long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);


	try {
		std::vector<Server>::iterator	it;
		for (it = servers.begin(); it != servers.end(); ++it) {
			it->setSocket();
			if (it->getSocket() == -1) {
				throw "Error SetSocket: ";
			}
			it->setSockAddr();
			it->bindSocket();
			it->listenSocket();
			it->addEpollCtl(epoll_fd);
		}
	}
	catch (std::exception &e) {
		close(epoll_fd);
		std::cerr << RED << e.what() << RESET << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	std::vector<Server>::iterator	it;
	for (it = servers.begin(); it != servers.end(); ++it) {
		std::cout << MAGENTA << it->getName() << " " << it->getSocket() << RESET << std::endl;
	}
	// std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
	
	std::string hello =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 12\r\n"
    "\r\n"
    "Hello world!\n";

	#define MAX_EVENTS 10
	while (g_runWebserv) {

		struct epoll_event	events[MAX_EVENTS];
		int	n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		for (int i = 0; i < n; i++) {
			if (Server::isServerSocket(events[i].data.fd, servers) && (events[i].events & EPOLLIN)) {
				Server::acceptClient(events[i].data.fd, servers, epoll_fd);
			}
			else if (Client::isClientSocket(events[i].data.fd, servers) && (events[i].events & EPOLLIN)){
				//JE DOIS LIRE et attention si 0 des la premiere lecture ON FERME TOUUUUT
				char	buffer[30000];
				if (recv(events[i].data.fd, buffer, 30000, 0) == 0) {
					std::cout << BLUE"CLOSING CLIENT"RESET << std::endl;
					Server::closingClient(epoll_fd, events[i].data.fd, servers);
					continue;
				}
				printf("%s", buffer);
				memset(buffer, 0, sizeof(buffer));
				while (recv(events[i].data.fd, buffer, 30000, 0) > 0) {
					printf("%s", buffer);
					memset(buffer, 0, sizeof(buffer));
				}
				std::cout << std::endl;
				events[i].events = EPOLLOUT | EPOLLET;// je dois utiliser CTL
				if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) < 0) {
					std::cerr << RED"Error epoll_ctl: "RESET << std::strerror(errno) << std::endl;
					//detruire le client socket ?
					Server::closingClient(epoll_fd, events[i].data.fd, servers);
				}

			}
			else if (Client::isClientSocket(events[i].data.fd, servers) && (events[i].events & EPOLLOUT)) {
				send(events[i].data.fd, hello.c_str(), hello.size(), 0);
				std::cout << RED"TEST========"RESET << std::endl;
				events[i].events = EPOLLIN | EPOLLET;// je dois utiliser CTL
				if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) < 0) {
					std::cerr << RED"Error epoll_ctl: "RESET << std::strerror(errno) << std::endl;
					//detruire le client socket ?
					Server::closingClient(epoll_fd, events[i].data.fd, servers);
				}
			}
		}
	}
	Server::closeAllSocket(epoll_fd, servers);
	std::cout << RED"[INFO] Server(s) Down"RESET << std::endl;
	/*	add server to epoll with epoll_ctl	*/
}