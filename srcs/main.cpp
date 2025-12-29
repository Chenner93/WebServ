#include <webserv.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <Config.hpp>
#include <CGI.hpp>
#include<../includes/Request/Request.hpp>
#include<../includes/Request/Response.hpp>
#include <sys/socket.h>

bool	g_runWebserv = true;

void	closeWebserv(int sig) {
	(void)sig;
	g_runWebserv = false;
	std::cout << std::endl << RED "[INFO] Shutting Down Server(s)..." RESET << std::endl;
}

int main(int ac, char **av)
{
	signal(SIGINT, closeWebserv);

	std::string config_file = (ac > 1) ? av[1] : "./Configuration_Files/DefaultWebserv.conf";

	Config config;
	if (!config.parseConfigFile(config_file))
	{
		std::cerr << RED "Error: Failed to parse configuration file" RESET << std::endl;
		return 1;
	}

	std::vector<Server> servers;
	std::vector<Client> clients;
	clients.reserve(10);

	const std::vector<ServerConfig>& server_configs = config.getServers();
	if (server_configs.empty())
	{
		std::cerr << RED "Error: No servers defined in configuration" RESET << std::endl;
		return 1;
	}

	servers.reserve(server_configs.size());

	// --- initialisation des serveurs ---
	for (size_t i = 0; i < server_configs.size(); ++i)
	{
		Server server;
		server.initServer(server_configs[i]);
		// server.printServerInfo(); NO
		servers.push_back(server);
	}

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		std::cerr << RED "Error: " << std::strerror(errno) << RESET << std::endl;
		exit(EXIT_FAILURE);
	}

	try
	{
		for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
		{
			it->setSocket();
			if (it->getSocket() == -1)
				throw "Error SetSocket: ";

			it->setSockAddr();
			it->bindSocket();
			it->listenSocket();
			it->addEpollCtl(epoll_fd);

			// std::cout << GREEN << "[LISTENING] " << it->getName()
			// 		  << " on port " << it->getPort()
			// 		  << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << e.what() << std::strerror(errno) << RESET << std::endl;
		g_runWebserv = false;
	}
	time_t		lastTimeoutCheck = time(NULL);
	const int	TIMEOUT_CHECK_INTERVAL = 5; // secondes

	while (g_runWebserv)
	{
		//check if TimeOut
		time_t	currentTime = time(NULL);
		if (difftime(currentTime, lastTimeoutCheck) >= TIMEOUT_CHECK_INTERVAL) {
			Client::checkTimeoutClients(clients, epoll_fd);
			lastTimeoutCheck = currentTime;
		}

		struct epoll_event events[MAX_EVENTS];
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);

		if (n < 0)
		{
			if (errno == EINTR)
				continue; // Interrupted by signal, retry
			std::cerr << RED "Error epoll_wait: " RESET << std::strerror(errno) << std::endl;
			break;
		}
	
		for (int i = 0; i < n; i++)
		{

			if (Server::isServerSocket(events[i].data.fd, servers) && (events[i].events & EPOLLIN))
			{
				Client::acceptClient(events[i].data.fd, servers, clients, epoll_fd);
				continue;
			}

			Client &client = Client::getClient(events[i].data.fd, clients);
			

			if (client.isCGI() == true) {
				client._CGI->CGIEvent(epoll_fd, clients, events[i], servers);
				
				CGI	&Cgi = *client._CGI;
				if (Cgi.getState() == CGI_END || Cgi.getState() == CGI_ERR) {
					if (Cgi.getState() == CGI_ERR) {
						if (Cgi.getPid() > 0)
							kill(Cgi.getPid(), SIGKILL);
					}
					if (Cgi.getState() == CGI_ERR && events[i].data.fd == client.getSocket()) {
						std::string patate = CGI::sendError(Cgi.getErrCgi(), "Internal Server Error", *client.getPtrServer());//+ in cgi errMessage;
						send(events[i].data.fd, patate.c_str(), patate.length(), 0);
						Client::closingClient(epoll_fd, events[i].data.fd, clients);
					}
					else if (Cgi.getState() == CGI_END)
						Client::closingClient(epoll_fd, events[i].data.fd, clients);
				}
				continue ;
			}

			if (Client::isClientSocket(events[i].data.fd, clients) && (events[i].events & EPOLLIN))
			{
				Client::epollinEvent(clients, events[i], epoll_fd);
			}
			else if (Client::isClientSocket(events[i].data.fd, clients) && (events[i].events & EPOLLOUT))
			{
				try {
					Client::epolloutEvent(clients, events[i], epoll_fd);
				}
				catch (const std::exception &e) {
					std::cerr << RED << "Bad Request: " << e.what() << RESET << std::endl;
				}
			}
		}
	}

	Server::closeAllSocket(epoll_fd, servers, clients);
	std::cout << RED "[INFO] Server(s) Down" RESET << std::endl;
}



