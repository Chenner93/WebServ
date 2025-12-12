#include <CGI.hpp>

CGI::CGI () {
	std::cout << "Constructor CGI Called" << std::endl;
	_pid = -2;

	_socket[0] = -2;
	_socket[1] = -2;
}

CGI::CGI(const std::string& cgi_path, const std::string& script_path) {
	std::cout << "Constructor CGI Called + Data" << std::endl;
	_cgi_path = cgi_path;
	_script_path = script_path;
	_pid = -2;

	_socket[0] = -2;
	_socket[1] = -2;
	bytesSend = 0;
}

CGI::~CGI() {
	std::cout << "Destructor CGI Called" << std::endl;
}

/*	SETTER	*/

void	CGI::setSocketVector() {
//Mettre un catch a ce throw

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _socket) == -1)
	{
		throw "Error: socketpair CGI";
	}
}

void	CGI::setState(std::string method) {
	if (method == "GET") {
		state = CGI_READING_OUTPUT;
	}
	else if (method == "POST") {
		state = CGI_WRITING_BODY;
	}
}

void	CGI::setFork() {
	
	this->_pid = fork();
	if (this->_pid == -1) {
		throw "Error: fork";
	}
}

void	CGI::setDup2() {
	dup2(getSocketParent(), STDIN_FILENO);
	dup2(getSocketParent(), STDOUT_FILENO);
	close(getSocketParent());
	close(getSocketChild());
}

void	CGI::setEpoll(int epoll_fd, std::vector<Client> &clients, struct epoll_event &event) {
	
	if (state == CGI_READING_OUTPUT) {
		event.events = EPOLLIN;
	}
	else if (state == CGI_WRITING_BODY) {
		event.events = EPOLLOUT;
	}

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->getSocketParent(), &event) < 0) {
		std::cerr << RED "Error: epoll_ctl in CGI: " RESET << std::strerror(errno) << std::endl;
		//kill child, send right error wait pid pour zombie;
		Client::closingClient(epoll_fd, event.data.fd, clients);
	}
	shutdown(this->getSocketParent(), SHUT_WR);

}

/*	GETTER	*/

std::string	CGI::getScriptPath() {
	return this->_script_path;
}

std::string	CGI::getCgiPath() {
	
	return this->_cgi_path;
}

int		CGI::getPid() const {
	return _pid;
}

int		CGI::getSocketParent() const {
	return _socket[0];
}

int		CGI::getSocketChild() const {
	return _socket[1];
}

int		CGI::getState() const {
	return state;
}

/*	UTILS	*/

bool	CGI::checkSocket(int fd) {
	if (_socket[0] == fd || _socket[1] == fd)
		return true;
	return false;
}

void	CGI::execCGI(Request *httpRequest) {

	(void)httpRequest;
	std::string	path_str = this->getCgiPath();
	std::string	scriptPath_str = this->getScriptPath();
	const char	*path = path_str.c_str();
	const char	*scriptPath = scriptPath_str.c_str();
	char	*av[] = {
		(char *)path,
		(char *)scriptPath,
		NULL
	};

	extern char **environ;
	execve(path, av, environ);
	std::cerr << RED "BLOOOOOOOOOOOOOOOOOOOOOOOOOOPSYYYYYYYYYYYYYYYYYYYYY" RESET << std::endl;
}

void	CGI::CGIEvent(int &epoll_fd, std::vector<Client> &clients, struct epoll_event &event) {

	Client &client = Client::getClient(event.data.fd, clients);

	if (_pid == -2) {
		//prep for child && add to epoll;
		this->setSocketVector();
		this->setState(client._requestParser->getMethod());
		this->setFork();
		if (this->getPid() == 0) {
			this->setDup2();
			this->execCGI(client._requestParser);
			exit(0);//exit de securiter
		}
		else {
			close(this->getSocketChild());
			this->setEpoll(epoll_fd, clients, event);
		}
		return ;
	}

	if (event.events & EPOLLIN) {
		//lecture of CGI until bytesread == 0
		char buffer[B_READ + 1];
		memset(buffer, 0, sizeof(buffer));
		ssize_t bytesread = recv(event.data.fd, buffer, B_READ, 0);

		if (bytesread <= 0)//Attention gerer si ==0 ou < 0
		{
			// std::cout << BLUE "END CGI" RESET << std::endl;
			close(this->getSocketParent());
			this->state = CGI_DONE;
			event.events = EPOLLOUT;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event);
			// Client::closingClient(epoll_fd, event.data.fd, clients);
			return;
		}
		std::string buff = buffer;
		this->_bodyCgi += buff;
	}
	else if (event.events & EPOLLOUT && this->state == CGI_WRITING_BODY) {
		//Send body to CGI then change to EPOLLIN
	}
	else if (event.events & EPOLLOUT && this->state == CGI_DONE) {
		//send response body mais on a pas encore send le header
		size_t	bytesToSend = B_SEND;
		if (bytesSend + B_SEND > this->_bodyCgi.size()) {
			bytesToSend = this->_bodyCgi.size() - bytesSend;
		}
		ssize_t	bSend = 0;
		bSend = send(event.data.fd, this->_bodyCgi.c_str() + bytesSend, bytesToSend, 0);
		if (bSend < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
				// Erreur douce, on retente apres
				return ;
			}
			std::cerr << RED "Error send: " RESET << std::strerror(errno) << std::endl;
			//close le client, erreur grave && kill child;
			Client::closingClient(epoll_fd, event.data.fd, clients);
		}
		bytesSend += bSend;
	}
}