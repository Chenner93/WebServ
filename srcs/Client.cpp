#include <Client.hpp>
#include <Server.hpp>


Client::Client() {
	std::cout << "Constructor Client Called" << std::endl;
	_socket = -1;
	_server = 0;
	_addrlen = sizeof(_addr);
}

Client::~Client() {
	std::cout << "Destructor Client Called" << std::endl;
}

Client::Client(const Client& copy) {
	std::cout << CYAN "Copy Client Called" RESET << std::endl;
	_socket = copy.getSocket();
	_server = copy.getPtrServer();
}

Client&	Client::operator = (const Client& src) {
	std::cout << CYAN "Ope = Client Called" RESET << std::endl;
	_socket = src.getSocket();
	_server = src.getPtrServer();
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
	clients.erase(it);
	std::cout << "Clients size = " << clients.size() << std::endl;
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
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, client.getSocket(), &event) < 0) {
		close(client.getSocket());
		std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
		return ;
	}
	clients.push_back(client);
}