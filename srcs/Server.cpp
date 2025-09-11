#include <Server.hpp>
#include<../includes/Request/Request.hpp>

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
	std::cout << BLUE "Copy Server Called" RESET << std::endl;

	_name = copy.getName();
	_ip = copy.getIp();
	_port = copy.getPort();
}

Server&	Server::operator = (const Server& src) {
	std::cout << "Ope = Server Called" << std::endl;
	if (this == &src)
		return *this;
	_name = src.getName();
	_ip = src.getIp();
	_port = src.getPort();
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
	std::cout << GREEN "Server Set" RESET << std::endl;
	_name = name;
	_ip = ip;
	_port = port;
}

void	Server::setSocket() {
	_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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

void	Server::closeAllSocket(int epfd, std::vector<Server> &servers, std::vector<Client> &clients) {
	{
		std::vector<Server>::iterator	it;
		for (it = servers.begin(); it != servers.end(); ++it)
			close(it->getSocket());
	}
	{
		std::vector<Client>::iterator	it;
		for (it = clients.begin(); it != clients.end(); ++it) {
			close(it->getSocket());
			delete it->getRequest();
		}
	}
	close(epfd);
}

  /********* */	
 /*	DEBUG	*/



//  request 

void Server::handleClientRequest(Client &client) {
    char buffer[4096];
    ssize_t bytes_received = recv(client.getSocket(), buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        // Gestion de déconnexion
        return;
    }
    buffer[bytes_received] = '\0';
    std::string rawRequest(buffer);

    try {
        Request request(rawRequest);
        request.parse_url();
         request.print_request(request); 
      
    } catch (const std::exception &e) {
        std::cerr << "Bad Request: " << e.what() << std::endl;
    }
}

