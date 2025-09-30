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
#include <sys/epoll.h>	// epoll_create(), epoll_ctl(), etc. (Linux spécifique)
#include <sys/socket.h>	// socket(), bind(), listen(), accept()
#include <unistd.h>		// POSIX : read(), write(), close()
#include <fcntl.h>
#include <stdio.h>
#include <Config.hpp>
#include"Request/Request.hpp"

class	Server {

	private:
		std::string			_name;
		std::string			_ip;
		int					_port;

		int					_socket;
		struct sockaddr_in	_addr;
		int					_addrlen;

		// Variables de configuration copiées depuis ServerConfig
		std::map<int, std::string> _error_pages;		// Codes d'erreurs -> Pages d'erreur
		size_t _client_max_body_size;					// Taille maximale du corps de la requête


		// Variables de Location copiées directement (pour chaque location)
		std::vector<std::string> _location_paths;		// Chemins des locations
		std::vector<std::vector<std::string> > _allow_methods_per_location;	// Méthodes autorisées par location
		std::vector<std::string> _roots;				// Racines des documents par location
		std::vector<std::string> _redirects;			// URLs de redirection par location
		std::vector<bool> _autoindex_flags;				// Flags autoindex par location
		std::vector<std::string> _index_files;			// Fichiers d'index par location
		std::vector<std::string> _upload_paths;			// Répertoires d'upload par location
		std::vector<bool> _upload_enabled_flags;		// Flags upload par location
		std::vector<std::map<std::string, std::string> > _cgi_configs;// Configs CGI par location

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

		/* GETTERS pour toutes les variables de configuration */
		const std::map<int, std::string>&	getErrorPages() const;
		size_t								getClientMaxBodySize() const;

		// Getters pour les locations (par index)
		size_t				getLocationCount() const;
		const std::string&	getLocationPath(size_t index) const;
		const std::vector<std::string>&	getAllowMethods(size_t index) const;
		const std::string&	getRoot(size_t index) const;
		const std::string&	getRedirect(size_t index) const;
		bool				getAutoindex(size_t index) const;
		const std::string&	getIndexFile(size_t index) const;
		const std::string&	getUploadPath(size_t index) const;
		bool				getUploadEnabled(size_t index) const;
		const std::map<std::string, std::string>&	getCgiConfig(size_t index) const;

		/*	SETTER	*/
		void	setServer(std::string name, std::string ip, int port);
		void	setSocket();
		void	setSockAddr();

		/* SETTERS pour toutes les variables de configuration */
		void addErrorPage(int error_code, const std::string& error_page);
		void setClientMaxBodySize(size_t max_size);

		// Setters pour ajouter une location complète
		void	addLocation(const std::string& path, 
						const std::vector<std::string>& allow_methods,
						const std::string& root,
						const std::string& redirect,
						bool autoindex,
						const std::string& index_file,
						const std::string& upload_path,
						bool upload_enabled,
						const std::map<std::string, std::string>& cgi_config);

		// Clear toutes les locations
		void	clearLocations();

		void		bindSocket();
		void		listenSocket();
		void		addEpollCtl(int epfd);

		/* NOUVELLE FONCTION : Initialisation depuis Config */
		void initServer(const ServerConfig& config);

		/* NOUVELLES FONCTIONS : Gestion des locations et erreurs */
		int findLocationIndex(const std::string& path) const;
		std::string getErrorPage(int error_code) const;
		bool isMethodAllowed(const std::string& path, const std::string& method) const;

		static bool	isServerSocket(int fd, std::vector<Server> &server);
		static void closeAllSocket(int epfd, std::vector<Server> &servers, std::vector<Client> &clients);

		/*	DEBUG	*/
		void	printServerInfo() const;
	};