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
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->getSocketParent(), &event) < 0) {
			std::cerr << RED "Error epoll_ctl: " RESET << std::strerror(errno) << std::endl;
			//kill child, send right error wait pid pour zombie;
			Client::closingClient(epoll_fd, event.data.fd, clients);
		}
	}
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

/*	UTILS	*/

bool	CGI::checkSocket(int fd) {
	if (_socket[0] == fd || _socket[1] == fd)
		return true;
	return false;
}

void	CGI::execCGI(Request *httpRequest) {

	(void)httpRequest;
	//Preparer l'exec;

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
	}
	else if (event.events & EPOLLOUT) {
		//Send body to CGI then change to EPOLLIN
	}
}