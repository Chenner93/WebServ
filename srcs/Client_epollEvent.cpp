#include <Client.hpp>
#include <Server.hpp>
#include <CGI.hpp>

void Client::epollinEvent(std::vector<Client> &clients, struct epoll_event &event, int epoll_fd)
{
	char buffer[B_READ + 1];
	memset(buffer, 0, sizeof(buffer));

	ssize_t bytesread = recv(event.data.fd, buffer, B_READ, 0);
	if (bytesread == 0)
	{
		Client::closingClient(epoll_fd, event.data.fd, clients);
		return;
	}
	else if (bytesread < 0) {
		std::cerr << RED "Error recv: " RESET << std::strerror(errno) << std::endl;
		Client::closingClient(epoll_fd, event.data.fd, clients);
		return ;
	}

	// Trouver le bon client
	size_t i;
	for (i = 0; i < clients.size(); i++)
	{
		if (clients[i].getSocket() == event.data.fd)
			break;
	}

	// Ajouter les données reçues à la requête en cours
	clients[i].appendRequest(buffer, bytesread);
	std::string &req = *clients[i].getRequest();

	// Chercher la fin des headers
	size_t header_end = req.find("\r\n\r\n");

	if (header_end != std::string::npos)
	{
		// Vérifier s’il y a un Content-Length
		size_t pos = req.find("Content-Length:");
		size_t content_length = 0;

		if (pos != std::string::npos)
		{
			pos += 15; // longueur de "Content-Length:"
			while (pos < req.size() && (req[pos] == ' ' || req[pos] == '\t'))
				++pos;
			content_length = std::atoi(req.c_str() + pos);
		}

		size_t total_needed = header_end + 4 + content_length;

		if (content_length == 0)
		{
			// std::cout << YELLOW << "[DEBUG] No Content-Length found — treating as complete" << RESET << std::endl;
			event.events = EPOLLOUT;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event) < 0)
			{
				std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
				Client::closingClient(epoll_fd, event.data.fd, clients);
			}
			return;
		}

		// Si tout le corps a été reçu
		if (req.size() >= total_needed)
		{
			event.events = EPOLLOUT;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event) < 0)
			{
				std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
				Client::closingClient(epoll_fd, event.data.fd, clients);
			}
		}
	}
}

void Client::epolloutEvent(std::vector<Client> &clients, struct epoll_event &event, int &epoll_fd)
{
	Client &client = Client::getClient(event.data.fd, clients);

	client.ParseRequest(); 	//request parsing
	if (client.CheckCGI() == true) {
		return ;
	}
	client.ParseResponse(); //Prep response

	//envoie de la reponse step by step et si tout est envoyer reset
	client.sendResponse(clients, event, epoll_fd);
}