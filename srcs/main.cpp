#include <webserv.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <Config.hpp>

#include <sys/socket.h>

bool	g_runWebserv = true;

void	closeWebserv(int sig) {
	(void)sig;
	g_runWebserv = false;
	std::cout << std::endl << RED "[INFO] Shutting Down Server(s)..." RESET << std::endl;
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

	(void)av;

	Config config;
	config.parseConfigFile("./Configuration_Files/DefaultWebserv.conf");
	config.printConfig();
	signal(SIGINT, closeWebserv);

	//check arguments and parse config_file, return vector of Server
	std::vector<Server>	servers;
	std::vector<Client>	clients;
	clients.reserve(10);
	tmp_config(ac, servers);

	int	epoll_fd = epoll_create1(0);

	if (epoll_fd == -1) {
		std::cerr << RED "Error: " RESET << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

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
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello world!\n";

	#define MAX_EVENTS	10
	#define BYTESREAD	10
	while (g_runWebserv) {

		struct epoll_event	events[MAX_EVENTS];
		int	n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
		std::cout << CYAN << "nb of Events: " << n << RESET << std::endl;
		for (int i = 0; i < n; i++) {
			if (Server::isServerSocket(events[i].data.fd, servers) && (events[i].events & EPOLLIN)) {
				std::cout << GREEN "Creation client" RESET << std::endl;
				Client::acceptClient(events[i].data.fd, servers, clients, epoll_fd);
			}
			else if (Client::isClientSocket(events[i].data.fd, clients) && (events[i].events & EPOLLIN)){
				//JE DOIS LIRE et attention si 0 des la premiere lecture ON FERME TOUUUUT
				Client::epollinEvent(clients, events[i], epoll_fd);
			}
			else if (Client::isClientSocket(events[i].data.fd, clients) && (events[i].events & EPOLLOUT)) {
				// std::cout << GREEN "client et epollout" RESET << std::endl;

				/*		--NEXT STEP HERE--
					
					PARSE REQUEST HERE
					savoir si on doit GET/POST/DEL
					si autre chose renvoyer une erreur avec "405 Method Not Allowed"
					ou "400 Bad Request" pour une requete sans methode
					
				*/

				send(events[i].data.fd, hello.c_str(), hello.size(), 0);
				events[i].events = EPOLLIN | EPOLLET;// je dois utiliser CTL
				if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) < 0) {
					std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
					//detruire le client socket ?
					Client::closingClient(epoll_fd, events[i].data.fd, clients);
				}
			}
		}
		//Check if timeout ?
	}
	Server::closeAllSocket(epoll_fd, servers, clients);
	std::cout << RED "[INFO] Server(s) Down" RESET << std::endl;
}