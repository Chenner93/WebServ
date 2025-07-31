#pragma once

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#include <Client.hpp>

#include <vector>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <iostream>
#include <cerrno>
#include <sys/epoll.h>  // epoll_create(), epoll_ctl(), etc. (Linux spécifique)
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <unistd.h>     // POSIX : read(), write(), close()
#include <fcntl.h>
#include <stdio.h>

class	Server {

	private:
		std::string			_name;
		std::string			_ip;
		int					_port;

		int					_socket;
		struct sockaddr_in	_addr;
		int					_addrlen;
		std::vector<Client>	_clients;

	public:
		Server();
		~Server();
		Server(const Server& copy);
		Server &operator = (const Server& src);

		/*	GETTER	*/
		int					getPort() const;
		std::string			getName() const;
		std::string			getIp() const;
		int					getSocket() const;
		std::vector<Client>	getVectorClient() const;

		/*	SETTER	*/
		void	setServer(std::string name, std::string ip, int port);
		void	setSocket();
		void	setSockAddr();

		void		bindSocket();
		void		listenSocket();
		void		addEpollCtl(int epfd);
		void		acceptClient();

		static bool	isServerSocket(int fd, std::vector<Server> &server);
		static void	acceptClient(int fd, std::vector<Server> &server, int epfd);
		static void closeAllSocket(int epfd, std::vector<Server> &servers);

		/*	DEBUG	*/
	};