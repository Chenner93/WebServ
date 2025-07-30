#pragma once

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
// #include <webserv.hpp>
// #include <sys/socket.h>

class	Server {

	private:
		std::string	_name;
		std::string	_ip;
		int			_port;

	public:
		Server();
		~Server();
		Server(const Server& copy);
		Server &operator = (const Server& src);

		/*	GETTER	*/
		int			getPort();
		std::string	getName();
		std::string	getIp();

		/*	SETTER	*/
		void	setServer(std::string name, std::string ip, int port);
};