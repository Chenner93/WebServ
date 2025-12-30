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

void	CGI::execCGI(Request *httpRequest) {

	std::vector<std::string>	envVector;
	std::vector<char *>			envp;

	std::string	path_str = this->getCgiPath();
	std::string	scriptPath_str = this->getScriptPath();
	const char	*path = path_str.c_str();
	const char	*scriptPath = scriptPath_str.c_str();
	envVector.push_back("REQUEST_METHOD=" + httpRequest->getMethod());
	envVector.push_back("QUERY_STRING=" + httpRequest->getPathAfterSign());
	envVector.push_back("SCRIPT_FILENAME=" + this->getScriptPath());
	envVector.push_back("REDIRECT_STATUS=200");

	for (size_t i = 0; i < envVector.size(); i++)
		envp.push_back(const_cast<char *>(envVector[i].c_str()));
	
	envp.push_back(NULL);
	char	*av[] = {
		(char *)path,
		(char *)scriptPath,
		NULL
	};

	extern char **environ;
	if (execve(path, av, environ) < 0) {
		std::cerr << RED "Error: execve" RESET << std::endl;
	}
}