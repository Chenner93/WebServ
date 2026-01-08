#include <CGI.hpp>

CGI::CGI () {
	// std::cout << "Constructor CGI Called" << std::endl;
	_pid = -2;

	_socket[0] = -2;
	_socket[1] = -2;
	errCgi = 0;
	this->setState(CGI_NEW_EPOLL);

}

CGI::CGI(const std::string& cgi_path, const std::string& script_path) {
	// std::cout << CYAN "Constructor CGI Called + Data" RESET << std::endl;
	_cgi_path = cgi_path;
	_script_path = script_path;
	_pid = -2;

	_socket[0] = -2;
	_socket[1] = -2;
	bytesSend = 0;
	errCgi = 0;
	this->setState(CGI_NEW_EPOLL);
	if (access(script_path.c_str(), F_OK) == 0) {
		if (access(script_path.c_str(), X_OK) != 0) {
			this->setState(CGI_ERR, 403, "Forbidden: unauthorized execution");
		}
	}
	else {
		this->setState(CGI_ERR, 404);
	}
}

CGI::CGI(const CGI& copy) {

	state = copy.state;
	_cgi_path = copy._cgi_path;
	_script_path = copy._script_path;
	_pid = copy._pid;
	_socket[0] = copy._socket[0];
	_socket[1] = copy._socket[1];
	_bodyCgi = copy._bodyCgi;
	bytesSend = copy.bytesSend;
	errCgi = copy.errCgi;
	state = copy.state;
	message = copy.message;
}

CGI&	CGI::operator = (const CGI& src) {

	if (this != &src) {
		state = src.state;
		_cgi_path = src._cgi_path;
		_script_path = src._script_path;
		_pid = src._pid;
		_socket[0] = src._socket[0];
		_socket[1] = src._socket[1];
		_bodyCgi = src._bodyCgi;
		bytesSend = src.bytesSend;
		errCgi = src.errCgi;
		state = src.state;
		message = src.message;
	}
	return *this;
}

CGI::~CGI() {}

/*	SETTER	*/

void	CGI::setSocketVector() {

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _socket) == -1)
		throw std::runtime_error(std::string("CGI: socketpair failed: ") + std::strerror(errno));
}

void	CGI::setStateMethod(std::string method) {
	if (method == "GET") {
		state = CGI_READING_OUTPUT;
	}
	else if (method == "POST") {
		state = CGI_WRITING_BODY;
	}
}

void	CGI::setState(CGIState step) {
	state = step;
}

void	CGI::setState(CGIState step, int err) {
	state = step;
	this->errCgi = err;
}

void	CGI::setState(CGIState step, int err, std::string message) {
	state = step;
	this->errCgi = err;
	this->message = message;
}

void	CGI::setFork() {
	
	this->_pid = fork();
	if (this->_pid == -1) {
		throw "Error: fork";
	}
}

void	CGI::setEpoll(int epoll_fd, std::vector<Client> &clients, struct epoll_event &event, int socketClient) {
	
	if (state == CGI_READING_OUTPUT) {
		event.events = EPOLLIN;
		shutdown(this->getSocketParent(), SHUT_WR);
	}
	else if (state == CGI_WRITING_BODY) {
		event.events = EPOLLOUT;
	}

	event.data.fd = this->getSocketParent();
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socketClient, 0) < 0) {
		std::cerr << RED "Error: epoll_ctl in CGI: " RESET << std::strerror(errno) << std::endl;
		Client::closingClient(epoll_fd, event.data.fd, clients);
	}
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->getSocketParent(), &event) < 0) {
		std::cerr << RED "Error: epoll_ctl in CGI: " RESET << std::strerror(errno) << std::endl;
		Client::closingClient(epoll_fd, event.data.fd, clients);
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

int		CGI::getState() const {
	return state;
}

int		CGI::getErrCgi() const {
	return errCgi;
}

int*	CGI::getPtrErrCgi() {
	return &errCgi;
}

bool	CGI::hasError() {
	return WIFEXITED(errCgi) && WEXITSTATUS(errCgi) != 0;
}

std::string	CGI::getMessage() const {
	return this->message;
}

/*	UTILS	*/

bool	CGI::checkSocket(int fd) {
	if (_socket[0] == fd || _socket[1] == fd)
		return true;
	return false;
}

  /***********/
 /*	UTILS	*/
/*	********/



std::string	CGI::createScriptPath(const std::string root, const std::string path) {
	size_t	i = path.find("/", 1);

	if (i != std::string::npos)
		return root + path.substr(i);
	return "";
}

bool		CGI::isMethodAllowed(const std::vector<std::string> allowedMethod, std::string method) {
	std::vector<std::string>::const_iterator	it;
	for (it = allowedMethod.begin(); it != allowedMethod.end(); ++it) {
		if (*it == method)
			return true;
	}
	return false;
}

std::string CGI::sendError(int code, std::string msg, const Server &server)
{
	if (code == 405) {
		msg = "Method Not Allowed";
	}
	std::ostringstream response;
	std::string body;
	std::string ctype = "text/html";

	std::map<int, std::string> const &error_map = server.getErrorPages();
	std::map<int, std::string>::const_iterator it = error_map.find(code);
	std::string error_path;

	if (it != error_map.end())
		error_path = it->second; 

	if (!error_path.empty())
	{
		std::ifstream file(error_path.c_str(), std::ios::binary);
		if (file.is_open())
		{
			std::ostringstream buf;
			buf << file.rdbuf();
			body = buf.str();
			file.close();
			ctype = "text/html";
		}
	}
	if (body.empty())
	{
		std::ostringstream oss;
		oss << code;
		body = "<html><body><h1>" + oss.str() + " " + msg + "</h1></body></html>";
	}
    
	response << "HTTP/1.1 " << code << " " << msg << "\r\n"
			 << "Content-Type: " << ctype << "\r\n"
			 << "Content-Length: " << body.size() << "\r\n"
			 << "Connection: close\r\n\r\n"
			 << body;

	// std::cerr << RED << "[HTTP " << code << "] " << msg << RESET << std::endl;
	return response.str();
}
void	CGI::TimeOutCGI(int epoll_fd, int socketClient) {
	if (this->getState() != CGI_ERR) {
		struct epoll_event event;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, this->getSocketParent(), 0) < 0) {
			std::cerr << RED "Hmmmm ?: " RESET << std::strerror(errno) << std::endl;
		}
		close(this->getSocketParent());
		this->_socket[0] = -1;
		event.data.fd = socketClient;
		event.events = EPOLLOUT;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socketClient, &event) < 0) {
			std::cerr << RED "Hmmmm ?: " RESET << std::strerror(errno) << std::endl;
		}
		
	}
	this->setState(CGI_ERR, 504, "Gateway Timeout");
}