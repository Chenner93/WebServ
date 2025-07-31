#include <Client.hpp>
#include <Server.hpp>


Client::Client() {
	std::cout << "Constructor Client Called" << std::endl;
	_socket = -1;
	_server = 0;
}

Client::~Client() {
	std::cout << "Destructor Client Called" << std::endl;
}

Client::Client(const Client& copy) {
	std::cout << CYAN"Copy Client Called"RESET << std::endl;
	_socket = copy.getSocket();
}

Client&	Client::operator = (const Client& src) {
	std::cout << "Ope = Client Called" << std::endl;
	return *this;
}

  /********* */
 /*	SETTER	*/
/********* */

void	Client::setSocket(int fd) {
	_socket = fd;
}

void	Client::setServer(Server server) {
	_server = &server;
}

  /********* */
 /*	GETTER	*/
/********* */

int		Client::getSocket() const{
	return _socket;
}

  /********* */
 /*	STATIC	*/
/********* */

Client*	Client::getPtrClient(int fd, std::vector<Server> &server) {
	std::vector<Server>::iterator	it;

	for (it = server.begin(); it != server.end(); ++it) {
		std::vector<Client>	clients = it->getVectorClient();
		std::vector<Client>::iterator	itC;
		for (itC = clients.begin(); itC != clients.end(); ++itC) {
			if (itC->getSocket() == fd)
				return &(*itC);
		}
	}
	return 0;
}