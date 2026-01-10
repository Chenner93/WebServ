#include <CGI.hpp>

void	CGI::setDup2() {
	dup2(getSocketChild(), STDIN_FILENO);
	dup2(getSocketChild(), STDOUT_FILENO);
	close(getSocketParent());
	close(getSocketChild());
}

void	CGI::closeAllSocket(int &epoll_fd, std::vector<Server> &servers, std::vector<Client> &clients) {
	{//Closing fd server
		std::vector<Server>::iterator	it;
		for (it = servers.begin(); it != servers.end(); ++it) {
			close(it->getSocket());
		}
	}
	{//Closing fd client && free data
		std::vector<Client>::iterator	it;
		for (it = clients.begin(); it != clients.end(); ++it) {
			close(it->getSocket());
		}
	}
	close(epoll_fd);
}

void	CGI::ManageErrExecve(std::vector<Client> &clients) {

	{//free data
		std::vector<Client>::iterator	it;
		for (it = clients.begin(); it != clients.end(); ++it) {
			it->resetAll();
		}
	}
	exit(EXIT_FAILURE);
}

std::string	CGI::intToString(size_t n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

void	CGI::setEnvp(Request *httpRequest, std::vector<std::string> &envVector) {

	//BASIC
	envVector.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVector.push_back("SERVER_SOFTWARE=webserv/1.0");
	envVector.push_back("GATEWAY_INTERFACE=CGI/1.1");

	envVector.push_back("PATH_INFO=" + httpRequest->getPath());
	envVector.push_back("SCRIPT_NAME=" + httpRequest->getPath());
	envVector.push_back("SCRIPT_FILENAME=" + this->getScriptPath());
	envVector.push_back("REDIRECT_STATUS=200");
	envVector.push_back("REQUEST_METHOD=" + httpRequest->getMethod());

	if (httpRequest->getPathAfterSign().empty() == false)
		envVector.push_back("QUERY_STRING=" + httpRequest->getPathAfterSign());

	//Content-type
	std::string contentType;// = httpRequest->getHeader("Content-Type");
	if (contentType.empty())
		contentType = "application/x-www-form-urlencoded";
	envVector.push_back("CONTENT_TYPE=" + contentType);

	//content-lenght
	std::string body = httpRequest->getBody();
	envVector.push_back("CONTENT_LENGTH=" + CGI::intToString(body.length()));

	//Dir cookie
	envVector.push_back("TMPDIR=/home/ckenaip/42Projects/webserv/cookie");
	envVector.push_back("TMP=/home/ckenaip/42Projects/webserv/cookie");
	envVector.push_back("TEMP=/home/ckenaip/42Projects/webserv/cookie");

	//header http
	std::string cookie = httpRequest->getHeader("cookie");
	if (!cookie.empty()) {
	    envVector.push_back("HTTP_COOKIE=" + cookie);
	}

	std::string host = httpRequest->getHeader("host");
	if (!host.empty())
	    envVector.push_back("HTTP_HOST=" + host);

	std::string userAgent = httpRequest->getHeader("user-Agent");
	if (!userAgent.empty())
	    envVector.push_back("HTTP_USER_AGENT=" + userAgent);

}

void	CGI::execCGI(Request *httpRequest) {

	std::vector<std::string>	envVector;
	std::vector<char *>			envp;

	std::string	path_str = this->getCgiPath();
	std::string	scriptPath_str = this->getScriptPath();
	const char	*path = path_str.c_str();
	const char	*scriptPath = scriptPath_str.c_str();

	this->setEnvp(httpRequest, envVector);

	for (size_t i = 0; i < envVector.size(); i++)
		envp.push_back(const_cast<char *>(envVector[i].c_str()));
	
	envp.push_back(NULL);
	char	*av[] = {
		(char *)path,
		(char *)scriptPath,
		NULL
	};

	if (execve(path, av, envp.data()) < 0) {
		std::cerr << RED "Error: execve" RESET << std::endl;
	}
}
