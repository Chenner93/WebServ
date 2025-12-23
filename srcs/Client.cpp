#include <Client.hpp>
#include <Server.hpp>
#include <CGI.hpp>


Client::Client() {
	// std::cout << "Constructor Client Called" << std::endl;
	_socket = -1;
	_server = 0;
	_CGI = 0;

	// _addr
	_addrlen = sizeof(_addr);

	_request = 0;

	_keepAlive = true;
	// _responseToSend

	_requestParser = 0;
	_response = 0;
	_bytesSend = 0;

	_lastActivity = time(NULL);
}

Client::~Client() {
	// std::cout << "Destructor Client Called" << std::endl;
}

Client::Client(const Client& copy) {
	// std::cout << CYAN "Copy Client Called" RESET << std::endl;

	_socket = copy.getSocket();
	_server = copy.getPtrServer();
	_CGI = copy._CGI;

	_addr = copy._addr;
	_addrlen = copy._addrlen;

	_request = copy._request;
	
	_keepAlive = copy.getKeepAlive();
	_responseToSend = copy._responseToSend;

	_requestParser = copy._requestParser;
	_response = copy._response;
	_bytesSend = copy._bytesSend;

	_lastActivity = copy._lastActivity;
}

Client&	Client::operator = (const Client& src) {
	// std::cout << CYAN "Ope = Client Called" RESET << std::endl;
	if (this != &src) {
		_socket = src.getSocket();
		_server = src.getPtrServer();
		_CGI = src._CGI;
	
		_addr = src._addr;
		_addrlen = src._addrlen;

		_request = src._request;

		_keepAlive = src.getKeepAlive();
		_responseToSend = src._responseToSend;

		_requestParser = src._requestParser;
		_response = src._response;
		_bytesSend = src._bytesSend;
		
		_lastActivity = src._lastActivity;
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
	updateLastActivity();
}

void	Client::appendRequest(char buffer[B_READ + 1], ssize_t bytesread) {
	if (getRequest() == 0)
		_request = new std::string;
	_request->append(buffer, bytesread);
	updateLastActivity();
}

void	Client::updateLastActivity() {
	_lastActivity = time(NULL);
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

time_t	Client::getLastActivity() const {
	return _lastActivity;
}

bool	Client::isTimeOut() const {
	time_t	currentTime = time(NULL);
	return (difftime(currentTime, _lastActivity) > TIMEOUT_CLIENT);
}

ssize_t	Client::getLocationIndex() const {
	return this->_server->findBestLocationIndex(this->_requestParser->getPath());
}

  /********* */
 /*	STATIC	*/
/********* */

Client	&Client::getClient(int fd, std::vector<Client> &clients){
	
	std::vector<Client>::iterator	it;
	for (it = clients.begin(); it != clients.end(); ++it) {
		if (it->getSocket() == fd)
			return *it;
		//check fd CGI
		if (it->_CGI && it->_CGI->checkSocket(fd))
			return *it;
	}
	throw "Error getClient. Should never happens";
}

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
		if (it->getSocket() == fd || (it->_CGI && it->_CGI->checkSocket(fd) == true))
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
	if (it->isCGI()) {
		close(it->_CGI->getSocketParent());
	}
	it->resetAll();
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

void	Client::checkTimeoutClients(std::vector<Client> &clients, int &epoll_fd) {
	std::vector<Client>::iterator	it = clients.begin();
	
	while (it != clients.end()) {
		if (it->isTimeOut() == true) {
			std::cout << BLUE "Client timed out (socket:" << it->getSocket()
					<< ", inactive for " << difftime(time(NULL), it->getLastActivity())
					<< " seconds)" RESET << std::endl;
		
			int socketToClose = it->getSocket();

			if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socketToClose, 0) < 0) {
				std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
			}
			close(socketToClose);
			it->resetAll();
			it = clients.erase(it);
		} else {
		it++;
		}
	}
}			

void	Client::resetAll() {
	if (_request)
		delete _request;
	
	if (_requestParser)
		delete _requestParser;

	if (_response)
		delete _response;

	if (_CGI)
		delete _CGI;

	_response = 0;
	_request = 0;
	_requestParser = 0;
	_CGI = 0;
	_bytesSend = 0;
}

void	Client::ParseRequest() {
	if (this->_requestParser != 0)
		return ;

	this->_requestParser = new Request(*this->getRequest(), this->getPtrServer());
	// this->_requestParser->parse_url();
	// this->_requestParser->print_request(*this->_requestParser);
	
	// --- DEBUG MULTIPART ---
	const std::map<std::string, std::string> &headers = this->_requestParser->getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("content-type");

	if (it != headers.end() &&
		it->second.find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = Request::ParseBoundary(headers);
		if (boundary.empty())
			std::cerr << RED << "[DEBUG] Aucun boundary trouvé." << RESET << std::endl;
		else
		{
			// std::cout << YELLOW << "[DEBUG] Boundary détectée : "
			// << boundary << RESET << std::endl;

			std::vector<FormDataPart> parts =
				this->_requestParser->parseMultipartFormData(this->_requestParser->getBody(), boundary);

			// this->_requestParser->printFormDataParts(parts);
		}
	}
	else
	{
		// std::cout << YELLOW << "[DEBUG] Requête non multipart." << RESET << std::endl;
	}

	updateLastActivity();
}

void	Client::ParseResponse() {
	if (this->_response != 0 || this->CheckCGI() == true)
		return ; //already set

	this->_response = new Response();
	this->_responseToSend = this->_response->Methodes(*(this->_requestParser), *(this->getPtrServer()));
	updateLastActivity();
}

void	Client::sendResponse(std::vector<Client> &clients, struct epoll_event &event, int &epoll_fd) {

	size_t	bytesToSend = B_SEND;
	if (_bytesSend + B_SEND > this->_responseToSend.size())
		bytesToSend = this->_responseToSend.size() - _bytesSend;

	ssize_t	bSend = 0;
	bSend = send(event.data.fd, this->_responseToSend.c_str() + _bytesSend,
		bytesToSend, 0);

	if (bSend < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			// Erreur douce, on retente apres
			return ;
		}
		std::cerr << RED "Error send: " RESET << std::strerror(errno) << std::endl;
		//close le client, erreur grave;
		Client::closingClient(epoll_fd, event.data.fd, clients);
		return ;
	}

	_bytesSend += bSend;
	updateLastActivity();

	if (_bytesSend == this->_responseToSend.size()) {
		this->resetAll();
		event.events = EPOLLIN;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event) < 0)
		{
			std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
			Client::closingClient(epoll_fd, event.data.fd, clients);
		}
	}
}

bool	Client::isCGI() {
	if (_CGI)
		return true;
	return false;
}

bool	Client::setCgi(Server &server) {

	// check if in CGI 
	if (this->_requestParser->Python_Or_Php() == false)
		return false;

	const std::string &path = this->_requestParser->getPath();
	ssize_t	index = this->getLocationIndex();
	//get path to executable
	const std::map<std::string, std::string>&	cgiConfig = server.getCgiConfig(index);
	//get root
	const std::string							&root = server.getRoot(index);
	//get extension executable for cgiConfig
	const std::string							extension = Request::getExtension(path);

	std::string	cgiPath = cgiConfig.at(extension);
	std::string scriptPath = CGI::createScriptPath(root, path);

	this->_CGI = new CGI(cgiPath, scriptPath);
	std::cout << MAGENTA << cgiPath << std::endl;
	std::cout << scriptPath << RESET << std::endl;
	return true;
}

bool	Client::CheckCGI() {

	if (this->isCGI())
		return true;

	return this->setCgi(*this->getPtrServer());
}

