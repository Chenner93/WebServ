#include <Client.hpp>
#include <Server.hpp>
#include <CGI.hpp>


Client::Client() {
	std::cout << "Constructor Client Called" << std::endl;
	_socket = -1;
	_server = 0;
	_addrlen = sizeof(_addr);
	_request = 0;
	_keepAlive = true;
	_CGI = 0;
	_requestParser = 0;
}

Client::~Client() {
	std::cout << "Destructor Client Called" << std::endl;
}

Client::Client(const Client& copy) {
	std::cout << CYAN "Copy Client Called" RESET << std::endl;
	_socket = copy.getSocket();
	_server = copy.getPtrServer();
	_request = copy._request;
	_keepAlive = copy.getKeepAlive();
}

Client&	Client::operator = (const Client& src) {
	std::cout << CYAN "Ope = Client Called" RESET << std::endl;
	if (this != &src) {
		_socket = src.getSocket();
		_server = src.getPtrServer();
		_request = src._request;
		_keepAlive = src.getKeepAlive();
	}
	return *this;
}

  /********* */
 /*	SETTER	*/
/********* */

void	Client::setSocket(int fd) {
	_socket = fd;
}

void	Client::setServer(Server &server) {
	_server = &server;
}

void	Client::appendRequest(char buffer[B_READ + 1]) {
	std::string buff = buffer;
	if (getRequest() == 0)
		_request = new std::string;
	_request->append(buff);
}

  /********* */
 /*	GETTER	*/
/********* */

int		Client::getSocket() const {
	return _socket;
}

Server	*Client::getPtrServer() const {
	return _server;
}

struct sockaddr_in	Client::getAddr() const {
	return _addr;
}

int		Client::getAddrlen() const {
	return _addrlen;
}

bool	Client::getKeepAlive() const {
	return _keepAlive;
}

std::string	*Client::getRequest() const {
	return _request;
}

//static
Client	&Client::getClient(int fd, std::vector<Client> &clients){
	
	std::vector<Client>::iterator	it;
	for (it = clients.begin(); it != clients.end(); ++it) {
		if (it->getSocket() == fd)
			return *it;
	}
	throw "Error getClient. Should never happens";
}

  /********* */
 /*	STATIC	*/
/********* */

bool	Client::isClientSocket(int fd, std::vector<Client> &clients) {

	std::vector<Client>::iterator	it;
	for (it = clients.begin(); it != clients.end(); ++it) {
		if (it->getSocket() == fd)
			return true;
	}
	return false;
}

void	Client::closingClient(int epfd, int fd, std::vector<Client> &clients) {

	std::vector<Client>::iterator	it;

	for (it = clients.begin(); it != clients.end(); ++it) {
		if (it->getSocket() == fd)
			break;
	}
	if (it == clients.end()) {
		std::cerr << RED "Error closingClient:" RESET << "Should never happens" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0) < 0) {
		std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
	}
	close(fd);
	if (it->_request)
		delete it->_request;
	clients.erase(it);
	std::cout << BLUE "client ERASEEEEED" RESET << std::endl;
}

void	Client::acceptClient(int fd, std::vector<Server> &servers, std::vector<Client> &clients, int epfd) {

	size_t	i;	//Got the right server ;
	for (i = 0; i < servers.size(); i++) {
		if (servers[i].getSocket() == fd)
			break ;
	}
	if (servers[i].getSocket() != fd) {
		std::cerr << RED"Erreur Server::acceptClient: Cannot happen !" << std::endl;
		return ;
	}
	Client	client;
	client.setServer(servers[i]);
	client.setSocket(::accept(fd, (struct sockaddr *)&client._addr, (socklen_t *)&client._addrlen));
	if (client.getSocket() < 0) {
		std::cerr << RED "Error accept: " RESET << std::strerror(errno) << std::endl;
		return ;
	}
	int	flags =fcntl(client.getSocket(), F_GETFL, 0);
	fcntl(client.getSocket(), F_SETFL, O_NONBLOCK | flags);
	struct epoll_event	event;
	event.data.fd = client.getSocket();
	event.events = EPOLLIN;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, client.getSocket(), &event) < 0) {
		close(client.getSocket());
		std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
		return ;
	}
	clients.push_back(client);
}

void	Client::freeRequest() {
	delete _request;
	_request = 0;
}


void Client::epollinEvent(std::vector<Client> &clients, struct epoll_event &event, int epoll_fd)
{
	char buffer[B_READ + 1];
	memset(buffer, 0, sizeof(buffer));

	ssize_t bytesread = recv(event.data.fd, buffer, B_READ, 0);
	if (bytesread <= 0)
	{
		std::cout << BLUE "CLOSING CLIENT" RESET << std::endl;
		Client::closingClient(epoll_fd, event.data.fd, clients);
		return;
	}

	// Trouver le bon client
	size_t i;
	for (i = 0; i < clients.size(); i++)
	{
		if (clients[i].getSocket() == event.data.fd)
			break;
	}

	// Ajouter les données reçues à la requête en cours
	clients[i].appendRequest(buffer);
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
			std::cout << YELLOW << "[DEBUG] No Content-Length found — treating as complete" << RESET << std::endl;
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
			std::cout << MAGENTA << "[DEBUG] Request complete (" << req.size()
					  << "/" << total_needed << " bytes)" << RESET << std::endl;

			event.events = EPOLLOUT;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event) < 0)
			{
				std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
				Client::closingClient(epoll_fd, event.data.fd, clients);
			}
		}
		else
		{
			std::cout << CYAN << "[DEBUG] Partial body received (" << req.size()
					  << "/" << total_needed << " bytes)" << RESET << std::endl;
		}
	}
	else
	{
		std::cout << CYAN << "[DEBUG] Waiting for headers..." << RESET << std::endl;
	}
}

// void Client::epolloutEvent(std::vector<Client> &clients, struct epoll_event &event, int epoll_fd)
// {
// 	Client &client = Client::getClient(events[i].data.fd, clients);

// }
