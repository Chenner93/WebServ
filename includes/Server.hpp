#pragma once

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
};