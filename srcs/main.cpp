#include <webserv.hpp>
#include <Server.hpp>
#include <Client.hpp>


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
	std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

	#define MAX_EVENTS 10
	while (g_runWebserv) {

		struct epoll_event	events[MAX_EVENTS];
		int	n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		for (int i = 0; i < n; i++) {
			Client	*client = Client::getPtrClient(events[i].data.fd, servers);
			if (Server::isServerSocket(events[i].data.fd, servers) && (events[i].events & EPOLLIN)) {
				Server::acceptClient(events[i].data.fd, servers, epoll_fd);
			}
			else if (client && events[i].events & EPOLLIN){
				//JE DOIS LIRE et attention si 0 des la premiere lecture ON FERME TOUUUUT
				char	buffer[30000];
				while (recv(client->getSocket(), buffer, 30000, 0) > 0) {
					printf("%s", buffer);
				}
				std::cout << std::endl;
				events[i].events = EPOLLIN | EPOLLOUT;
			}
			else if (client && events[i].events & EPOLLOUT) {
				//Je dois REPONDRE
			}
		}
	}
	Server::closeAllSocket(epoll_fd, servers);
	std::cout << RED"[INFO] Server(s) Down"RESET << std::endl;
	/*	add server to epoll with epoll_ctl	*/
}