#include <Server.hpp>

Server::Server() {
	std::cout << "Constructor Server Called" << std::endl;
	_name = "ServerTest";
	_ip = "127.0.0.1";
	_port = 8080;
	_socket = -1;
}

Server::~Server() {
	std::cout << "Destructor Server Called" << std::endl;
}

Server::Server(const Server& copy) {
	std::cout << BLUE"Copy Server Called"RESET << std::endl;

	_name = copy.getName();
	_ip = copy.getIp();
	_port = copy.getPort();
}

Server&	Server::operator = (const Server& src) {
	std::cout << "Ope = Server Called" << std::endl;
	return *this;
}

  /********* */
 /*	GETTER	*/
/********* */

int	Server::getPort() const{
	return _port;
}

std::string	Server::getName() const{
	return _name;
}

std::string	Server::getIp() const{
	return _ip;
}

int	Server::getSocket() const{
	return _socket;
}

  /********* */
 /*	SETTER	*/
/********* */

void	Server::setServer(std::string name, std::string ip, int port) {
	std::cout << GREEN"Server Set"RESET << std::endl;
	_name = name;
	_ip = ip;
	_port = port;
}

void	Server::setSocket() {
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	int	opt = 1;
	if (_socket != -1 && (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0) {
		throw "Error SetsockOPT: ";
	}
}

void	Server::setSockAddr() {

	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(_port);
	std::memset(_addr.sin_zero, '\0', sizeof(_addr.sin_zero));
	_addrlen = sizeof(_addr);
}

void	Server::bindSocket() {
	if (::bind(_socket, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
		throw "Error bind: ";
}

void	Server::listenSocket() {
	if (::listen(_socket, SOMAXCONN) < 0) { //max kernel connexion waiting
		throw "Error listen: ";
	}
}

void	Server::addEpollCtl(int epfd) {
	struct epoll_event	event;
	event.data.fd = _socket;
	event.events = EPOLLIN;
	if (::epoll_ctl(epfd, EPOLL_CTL_ADD, _socket, &event) == -1) {
		throw "Error epoll_ctl: ";
	}
}

  /********* */	
 /*	STATIC	*/
/********* */

bool	Server::isServerSocket(int fd, std::vector<Server> &server) {
	std::vector<Server>::iterator	it;

	for (it = server.begin(); it != server.end(); ++it) {
		if (it->_socket == fd)
			return true;
	}
	return false;
}

void	Server::acceptClient(int fd, std::vector<Server> &servers, std::vector<Client> clients, int epfd) {

	int	i;	//Got the right server ;
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
	client.setSocket(::accept(fd, (struct sockaddr *)&servers[i]._addr, (socklen_t *)&servers[i]._addrlen));
	if (client.getSocket() < 0) {
		std::cerr << RED"Error accept: "RESET << std::strerror(errno) << std::endl;
		return ;
	}
	fcntl(client.getSocket(), F_SETFL, O_NONBLOCK);
	struct epoll_event	event;
	event.data.fd = client.getSocket();
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, client.getSocket(), &event) < 0) {
		close(client.getSocket());
		std::cerr << RED"Error epoll_ctl: "RESET << std::strerror(errno) << std::endl;
		return ;
	}
	clients.push_back(client);
}

// void	Server::closingClient(int epfd, int fd, std::vector<Server> &servers) {

// 	std::vector<Server>::iterator	it;

// 	for (it = servers.begin(); it != servers.end(); ++it) {
// 		std::vector<Client>::iterator	itC;
// 		for (itC = it->_clients.begin(); itC != it->_clients.end(); ++itC) {
// 			if (itC->getSocket() == fd) {
// 				epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0);
// 				close(fd);
// 				it->_clients.erase(itC);
// 				std::cout << BLUE"ERASEEEEEEEEEEED"RESET << std::endl;
// 				return ;
// 			}
// 		}
// 	}
// }

void	Server::closeAllSocket(int epfd, std::vector<Server> &servers, std::vector<Client> &clients) {
	{
		std::vector<Server>::iterator	it;
		for (it = servers.begin(); it != servers.end(); ++it)
			close(it->getSocket());
	}
	{
		std::vector<Client>::iterator	it;
		for (it = clients.begin(); it != clients.end(); ++it)
			close(it->getSocket());
	}
	close(epfd);
}

  /********* */	
 /*	DEBUG	*/
/********* */
