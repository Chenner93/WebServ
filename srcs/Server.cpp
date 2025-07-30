#include <Server.hpp>

Server::Server() {
	std::cout << "Constructor Server Called" << std::endl;
	_name = "ServerTest";
	_ip = "127.0.0.1";
	_port = 8080;
}

Server::~Server() {
	std::cout << "Destructor Server Called" << std::endl;
}

Server::Server(const Server& copy) {
	std::cout << "Copy Server Called" << std::endl;
}

Server::Server &operator = (const Server& src) {
	std::cout << "Ope = Server Called" << std::endl;
}

  /********* */
 /*	GETTER	*/
/********* */

int	Server::getPort() {
	return _port;
}

std::string	Server::getName() {
	return _name;
}

std::string	Server::getIp() {
	return _ip;
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
