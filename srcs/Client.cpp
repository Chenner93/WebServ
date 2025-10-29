#include <Client.hpp>
#include <Server.hpp>


Client::Client() {
	std::cout << "Constructor Client Called" << std::endl;
	_socket = -1;
	_server = 0;
	_addrlen = sizeof(_addr);
	_request = 0;
	_keepAlive = true;
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


// --- Client.cpp parse  complete line ---
static bool is_request_complete(const std::string& buf) {
    // 1) Besoin d'un bloc headers entier
    std::size_t head_end = buf.find("\r\n\r\n");
    if (head_end == std::string::npos) return false;

    // 2) Extraire les headers
    std::string head = buf.substr(0, head_end);
    // Chercher Content-Length et Transfer-Encoding
    std::string lower = head;
    for (size_t i=0;i<lower.size();++i) lower[i] = std::tolower(static_cast<unsigned char>(lower[i]));

    // 2.a) Chunked ?
    if (lower.find("transfer-encoding: chunked") != std::string::npos) {
        // Requête complète si on trouve la terminaison chunked minimale "0\r\n\r\n" APRES les headers
        std::size_t body_start = head_end + 4;
        std::size_t term = buf.find("\r\n0\r\n\r\n", body_start);
        if (term == std::string::npos) {
            // certains clients n'envoient pas l'extra \r\n avant "0"
            term = buf.find("0\r\n\r\n", body_start);
        }
        return (term != std::string::npos);
    }

    // 2.b) Content-Length ?
    std::size_t pos = lower.find("content-length:");
    if (pos != std::string::npos) {
        // récupère la valeur
        std::size_t eol = lower.find("\r\n", pos);
        std::string line = head.substr(pos, (eol==std::string::npos? head.size()-pos : eol-pos));
        std::size_t colon = line.find(':');
        size_t cl = std::strtoul(line.substr(colon+1).c_str(), 0, 10);

        std::size_t body_start = head_end + 4;
        return buf.size() >= body_start + cl;
    }

    // 2.c) Pas de body attendu (GET/HEAD sans CL/TE) → complet
    return true;
}


void	Client::epollinEvent(std::vector<Client> &clients, struct epoll_event &event, int epoll_fd) {

	char	buffer[B_READ + 1];
	memset(buffer, 0, sizeof(buffer));
	size_t bytesread = recv(event.data.fd, buffer, B_READ, 0);

	if (bytesread <= 0) {// SEPARER == 0 && < 0
		std::cout << BLUE "CLOSING CLIENT" RESET << std::endl;
		Client::closingClient(epoll_fd, event.data.fd, clients);
		return ;
	}

	size_t	i;
	for (i = 0; i < clients.size(); i++) {
		if (clients[i].getSocket() == event.data.fd)
			break ;

		// --- Client.cpp test debug ---
		clients[i].appendRequest(buffer);

// Nouveau test : la requête est-elle complète test debug ?
if (clients[i].getRequest() && is_request_complete(*clients[i].getRequest())) {
    std::cout << MAGENTA << "Request complete ✓" << RESET << std::endl;
    event.events = EPOLLOUT;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event) < 0) {
        std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
        Client::closingClient(epoll_fd, event.data.fd, clients);
    }
}

	}

	// if (bytesread > 0 && bytesread == B_READ) {
	// 	//will have to check if got all the requests (if request full then epollout)
	// 	clients[i].appendRequest(buffer);
	// 	memset(buffer, 0, sizeof(buffer));
	// }
	// else {
	// 	std::cout << MAGENTA << "Request client :\n" << *clients[i].getRequest() << RESET << std::endl;
	// 	event.events = EPOLLOUT;
	// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event) < 0) {
	// 		std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
	// 		Client::closingClient(epoll_fd, event.data.fd, clients);
	// 	}
	// }
	// std::cout << std::endl;

	// Ajouter les données reçues
	clients[i].appendRequest(buffer);

	// Vérifier si la requête est complète (présence de \r\n\r\n)
	if (clients[i].getRequest() && clients[i].getRequest()->find("\r\n\r\n") != std::string::npos) {
		// Requête complète ! Passer en mode écriture
		std::cout << MAGENTA << "Request complete:\n" << clients[i].getRequest() << RESET << std::endl;
		
		event.events = EPOLLOUT;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event) < 0) {
			std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
			Client::closingClient(epoll_fd, event.data.fd, clients);
		}
	}
	// Sinon, on attend plus de données (reste en EPOLLIN)
}

void	Client::freeRequest() {
	delete _request;
	_request = 0;
}