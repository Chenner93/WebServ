#include <webserv.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <Config.hpp>
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
		server.printServerInfo();
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

			std::cout << GREEN << "[LISTENING] " << it->getName()
					  << " on port " << it->getPort()
					  << RESET << std::endl;
		}
	}
	catch (const char *e)
	{
		close(epoll_fd);
		std::cerr << RED << e << std::strerror(errno) << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	catch (std::exception &e)
	{
		close(epoll_fd);
		std::cerr << RED << e.what() << std::strerror(errno) << RESET << std::endl;
		exit(EXIT_FAILURE);
	}

	/*		GIGA TEST CGI	*/

	// try
	// {
	// 	CGI	cgi;

	// 	cgi.setSocketVector();
	// }
	// catch (const char *e)
	// {
	// 	std::cerr << RED << e << std::strerror(errno) << RESET << std::endl;
	// }
	/*		END TEST CGI	*/


	while (g_runWebserv)
	{
		struct epoll_event events[MAX_EVENTS];
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 0);

		for (int i = 0; i < n; i++)
		{
			if (Server::isServerSocket(events[i].data.fd, servers) && (events[i].events & EPOLLIN))
			{
				std::cout << GREEN "Creation client" RESET << std::endl;
				Client::acceptClient(events[i].data.fd, servers, clients, epoll_fd);
			}
			else if (Client::isClientSocket(events[i].data.fd, clients) && (events[i].events & EPOLLIN))
			{
				Client::epollinEvent(clients, events[i], epoll_fd);
			}
			else if (Client::isClientSocket(events[i].data.fd, clients) && (events[i].events & EPOLLOUT))
			{
				Client &client = Client::getClient(events[i].data.fd, clients);

				try
				{
					std::cout << GREEN << "PASS IN MAIN MY PART" << RESET << std::endl;

					client._requestParser = new Request(*client.getRequest(), client.getPtrServer());

					client._requestParser->parse_url();
					client._requestParser->print_request(*client._requestParser);

					// --- DEBUG MULTIPART ---
					const std::map<std::string, std::string> &headers = client._requestParser->getHeaders();
					std::map<std::string, std::string>::const_iterator it = headers.find("content-type");

					if (it != headers.end() &&
						it->second.find("multipart/form-data") != std::string::npos)
					{
						std::string boundary = Request::ParseBoundary(headers);
						if (boundary.empty())
							std::cerr << RED << "[DEBUG] Aucun boundary trouvé." << RESET << std::endl;
						else
						{
							std::cout << YELLOW << "[DEBUG] Boundary détectée : "
									  << boundary << RESET << std::endl;

							std::vector<FormDataPart> parts =
								client._requestParser->parseMultipartFormData(client._requestParser->getBody(), boundary);

							client._requestParser->printFormDataParts(parts);
						}
					}
					else
					{
						std::cout << YELLOW << "[DEBUG] Requête non multipart." << RESET << std::endl;
					}

					// --- génération de la réponse ---
					
					Response response;

					std::string res = response.Methodes(*client._requestParser, *client.getPtrServer());
					send(events[i].data.fd, res.c_str(), res.size(), 0);
					client.freeRequest();
					delete client._requestParser;
				}
				catch (const std::exception &e)
				{
					std::cerr << RED << "Bad Request: " << e.what() << RESET << std::endl;
				}

				// repasser le client en lecture
				events[i].events = EPOLLIN;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) < 0)
				{
					std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
					Client::closingClient(epoll_fd, events[i].data.fd, clients);
				}
			}
		}
	}

	Server::closeAllSocket(epoll_fd, servers, clients);
	std::cout << RED "[INFO] Server(s) Down" RESET << std::endl;
}



