#include <CGI.hpp>

void	CGI::CGIEvent(int &epoll_fd, std::vector<Client> &clients, struct epoll_event &event, std::vector<Server> &servers) {

	Client &client = Client::getClient(event.data.fd, clients);

	// if (this->getState() == CGI_ERR)
	// 	return ;

	if (_pid == -2 && this->getState() != CGI_ERR) {
		//prep for child && add to epoll;
		this->setSocketVector();
		this->setStateMethod(client._requestParser->getMethod());
		this->setFork();
		if (this->getPid() == 0) {
			this->setDup2();
			CGI::closeAllSocket(epoll_fd, servers, clients);
			this->execCGI(client._requestParser);
			CGI::ManageErrExecve(clients);
		}
		else {
			this->setEpoll(epoll_fd, clients, event, client.getSocket());
			close(this->getSocketChild());
		}
		return ;
	}

	if (event.events & EPOLLIN && this->state == CGI_READING_OUTPUT) {
		//lecture of CGI until bytesread == 0
		char buffer[B_READ + 1];
		memset(buffer, 0, sizeof(buffer));
		ssize_t bytesread = recv(event.data.fd, buffer, B_READ, 0);

		if (bytesread == 0)
		{
			close(this->getSocketParent());
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, 0);	//suppress CGI fd from epoll
			this->state = CGI_DONE;
			event.events = EPOLLOUT;
			event.data.fd = client.getSocket();
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event)) {
				std::cerr << RED "Error: epoll_ctl in CGI: END of READ" RESET << std::strerror(errno) << std::endl;
				this->setState(CGI_ERR, 500);
			}
			return;
		}
		else if (bytesread < 0) {
			
			std::cerr << RED "Error recv: " RESET << std::strerror(errno) << std::endl;
			this->setState(CGI_ERR, 500);
			event.events = EPOLLOUT;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, &event)) {
				std::cerr << RED "Error: epoll_ctl in CGI" RESET << std::strerror(errno) << std::endl;
			}
			event.data.fd = client.getSocket();
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event)) {
				std::cerr << RED "Error: epoll_ctl in CGI" RESET << std::strerror(errno) << std::endl;
			}	
			return ;
		}
		std::string buff = buffer;
		this->_bodyCgi += buff;
	}
	else if (event.events & EPOLLOUT && this->state == CGI_WRITING_BODY) {
		//Send body to CGI then change to EPOLLIN
		size_t	bSend = 0;
		//maybe check et not all the body is send ?
		bSend = send(event.data.fd, client._requestParser->getBody().c_str(), client._requestParser->getBody().size(), 0);

		shutdown(this->getSocketParent(), SHUT_WR);// to do after sendind all info

		this->setState(CGI_READING_OUTPUT);
		event.events = EPOLLIN;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event) < 0) {
			std::cerr << RED "Error: epoll_ctl in CGI: " RESET << std::strerror(errno) << std::endl;
			this->setState(CGI_ERR);
		}
		return ;
	}
	else if (event.events & EPOLLOUT && this->state == CGI_DONE) {
		
		// 1- CHECKif Header&Body
		size_t	sep = _bodyCgi.find("\n\n");
		size_t	offset = 2;
		if (sep == std::string::npos) {
			sep = _bodyCgi.find("\r\n\r\n");
			offset = 4;
		}

		if (sep == std::string::npos) {
			std::cerr << RED "no headers CGI" << std::endl;
			this->setState(CGI_ERR, 500);
			return ;
		}

		// 2- Extraction Header&Body
		std::string cgi_headers = _bodyCgi.substr(0, sep);
    	std::string body = _bodyCgi.substr(sep + offset);

		// 3- Parser Status
   		int status = 200;
		size_t status_pos = cgi_headers.find("Status:");
		if (status_pos != std::string::npos) {
			status = atoi(cgi_headers.c_str() + status_pos + 8);
			size_t line_end = cgi_headers.find("\n", status_pos);
			if (line_end != std::string::npos)
				cgi_headers.erase(status_pos, line_end - status_pos + 1);
			else
				cgi_headers.erase(status_pos);
		}
		if ((status < 100 || status > 599) && ) {
			std::cerr << RED "Invalid CGI status: " << status << RESET << std::endl;
			this->setState(CGI_ERR, 500);
			return;
		}

		// 4- Construire réponse HTTP
		std::stringstream ss, ssStatus;
		ssStatus << status;
		std::string http_response = "HTTP/1.1 ";
		http_response += ssStatus.str();
		http_response += (status == 200 ? " OK" : " Error");
		http_response += "\r\n";
		http_response += cgi_headers;
		if (cgi_headers.find("Content-Length:") == std::string::npos) {
			ss << body.size();
			if (!cgi_headers.empty() && cgi_headers[cgi_headers.size() - 1] != '\n') {
    			http_response += "\r\n";
			}
			http_response += "Content-Length: " + ss.str();
		}
		http_response += "\r\n\r\n";	// Fin headers HTTP
		http_response += body;
		
		std::cout << MAGENTA << http_response << RESET << std::endl;
    	_bodyCgi = http_response;
    	this->state = CGI_SEND;
	}
	else if (event.events & EPOLLOUT && this->state == CGI_SEND) {

		size_t	bytesToSend = B_SEND;
		if (bytesSend + B_SEND > this->_bodyCgi.size()) {
			bytesToSend = this->_bodyCgi.size() - bytesSend;
		}
		ssize_t	bSend = 0;
		bSend = send(client.getSocket(), this->_bodyCgi.c_str() + bytesSend, bytesToSend, 0);
		if (bSend < 0) {
			std::cerr << RED "Error send: " RESET << std::strerror(errno) << std::endl;
			this->setState(CGI_ERR);
			return ;
		}
		bytesSend += bSend;
		if (bytesSend >= _bodyCgi.size()) {
			this->setState(CGI_END);
			event.data.fd = client.getSocket();
			event.events = EPOLLIN;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client.getSocket(), &event) < 0) {
				std::cerr << RED "Error: epoll_ctl in CGI: " RESET << std::strerror(errno) << std::endl;
			}
		}
	}
}