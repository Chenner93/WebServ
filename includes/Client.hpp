#pragma once

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define B_READ	10

#include <iostream>
#include <vector>
#include <cstring>
#include <cerrno>
#include <netinet/in.h>
#include <unistd.h>     // POSIX : read(), write(), close()
#include <stdlib.h>		// exit

class	Server;
class	Client {

	private:
		int					_socket;
		Server				*_server;
		
		struct sockaddr_in	_addr;
		int					_addrlen;

		std::string			*_request;

		bool				_keepAlive;

	public:
		Client();
		~Client();
		Client(const Client& copy);
		Client &operator = (const Client& src);

	/*	SETTER	*/
		void	setSocket(int fd);
		void	setServer(Server &server);

		void	appendRequest(char buffer[B_READ + 1]);

	/*	GETTER	*/
		int					getSocket() const;
		Server				*getPtrServer() const;
		int					getAddrlen() const;
		struct sockaddr_in	getAddr() const;
		std::string			*getRequest() const;
		bool				getKeepAlive() const;



	/*	STATIC	*/
		static bool	isClientSocket(int fd, std::vector<Client> &clients);
		static void closingClient(int epfd, int fd, std::vector<Client> &clients);
		static void	acceptClient(int fd, std::vector<Server> &servers, std::vector<Client> &clients, int epfd);
		static void	epollinEvent(std::vector<Client> &clients, struct epoll_event &event, int epoll_fd);

};