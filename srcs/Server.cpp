#include <Server.hpp>
#include<../includes/Request/Request.hpp>

Server::Server() {
	std::cout << "Constructor Server Called" << std::endl;
	_name = "ServerTest";
	_ip = "127.0.0.1";
	_port = 8080;
	_socket = -1;
	_client_max_body_size = 1048576; // 1MB par défaut
}

Server::~Server() {
	std::cout << "Destructor Server Called" << std::endl;
}

Server::Server(const Server& copy) {
	std::cout << BLUE "Copy Server Called" RESET << std::endl;

	_name = copy.getName();
	_ip = copy.getIp();
	_port = copy.getPort();

	_error_pages = copy.getErrorPages();
	_client_max_body_size = copy.getClientMaxBodySize();

	_location_paths = copy._location_paths;
	_allow_methods_per_location = copy._allow_methods_per_location;
	_roots = copy._roots;
	_redirects = copy._redirects;
	_autoindex_flags = copy._autoindex_flags;
	_index_files = copy._index_files;
	_upload_paths = copy._upload_paths;
	_upload_enabled_flags = copy._upload_enabled_flags;
	_cgi_configs = copy._cgi_configs;
}

Server&	Server::operator = (const Server& src) {
	std::cout << "Ope = Server Called" << std::endl;
	if (this == &src)
		return *this;
	_name = src.getName();
	_ip = src.getIp();
	_port = src.getPort();

	// Variables de configuration
	_error_pages = src.getErrorPages();
	_client_max_body_size = src.getClientMaxBodySize();

	// Toutes les variables de locations
	_location_paths = src._location_paths;
	_allow_methods_per_location = src._allow_methods_per_location;
	_roots = src._roots;
	_redirects = src._redirects;
	_autoindex_flags = src._autoindex_flags;
	_index_files = src._index_files;
	_upload_paths = src._upload_paths;
	_upload_enabled_flags = src._upload_enabled_flags;
	_cgi_configs = src._cgi_configs;
	return *this;
}

  /********* */
 /*	GETTER	*/
/********* */

int	Server::getPort() const{
	return _port;
}

std::string	Server::getName() const{
	return _name;
}

std::string	Server::getIp() const{
	return _ip;
}

int	Server::getSocket() const{
	return _socket;
}

/*********** GETTERS DE CONFIGURATION ***********/

const std::map<int, std::string>& Server::getErrorPages() const {
	return _error_pages;
}

size_t Server::getClientMaxBodySize() const {
	return _client_max_body_size;
}

// Getters pour les locations
size_t Server::getLocationCount() const {
	return _location_paths.size();
}

const std::string& Server::getLocationPath(size_t index) const {
	return _location_paths[index];
}

const std::vector<std::string>& Server::getAllowMethods(size_t index) const {
	return _allow_methods_per_location[index];
}

const std::string& Server::getRoot(size_t index) const {
	return _roots[index];
}

const std::string& Server::getRedirect(size_t index) const {
	return _redirects[index];
}

bool Server::getAutoindex(size_t index) const {
	return _autoindex_flags[index];
}

const std::string& Server::getIndexFile(size_t index) const {
	return _index_files[index];
}

const std::string& Server::getUploadPath(size_t index) const {
	return _upload_paths[index];
}

bool Server::getUploadEnabled(size_t index) const {
	return _upload_enabled_flags[index];
}

const std::map<std::string, std::string>& Server::getCgiConfig(size_t index) const {
	return _cgi_configs[index];
}

  /********* */
 /*	SETTER	*/
/********* */

void	Server::setServer(std::string name, std::string ip, int port) {
	std::cout << GREEN "Server Set" RESET << std::endl;
	_name = name;
	_ip = ip;
	_port = port;
}

void	Server::setSocket() {
	_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	int	opt = 1;
	if (_socket != -1 && (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0) {
		throw "Error SetsockOPT: ";
	}
}

void	Server::setSockAddr() {

	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(_port);
	std::memset(_addr.sin_zero, '\0', sizeof(_addr.sin_zero));
	_addrlen = sizeof(_addr);
}

void	Server::bindSocket() {
	if (::bind(_socket, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
		throw "Error bind: ";
}

void	Server::listenSocket() {
	if (::listen(_socket, SOMAXCONN) < 0) { //max kernel connexion waiting
		throw "Error listen: ";
	}
}

void	Server::addEpollCtl(int epfd) {
	struct epoll_event	event;
	event.data.fd = _socket;
	event.events = EPOLLIN;
	if (::epoll_ctl(epfd, EPOLL_CTL_ADD, _socket, &event) == -1) {
		throw "Error epoll_ctl: ";
	}
}

/*********** SETTERS DE CONFIGURATION ***********/

void Server::addErrorPage(int error_code, const std::string& error_page) {
	_error_pages[error_code] = error_page;
}

void Server::setClientMaxBodySize(size_t max_size) {
	_client_max_body_size = max_size;
}

void Server::addLocation(const std::string& path, 
						const std::vector<std::string>& allow_methods,
						const std::string& root,
						const std::string& redirect,
						bool autoindex,
						const std::string& index_file,
						const std::string& upload_path,
						bool upload_enabled,
						const std::map<std::string, std::string>& cgi_config) {
	_location_paths.push_back(path);
	_allow_methods_per_location.push_back(allow_methods);
	_roots.push_back(root);
	_redirects.push_back(redirect);
	_autoindex_flags.push_back(autoindex);
	_index_files.push_back(index_file);
	_upload_paths.push_back(upload_path);
	_upload_enabled_flags.push_back(upload_enabled);
	_cgi_configs.push_back(cgi_config);
}

void Server::clearLocations() {
	_location_paths.clear();
	_allow_methods_per_location.clear();
	_roots.clear();
	_redirects.clear();
	_autoindex_flags.clear();
	_index_files.clear();
	_upload_paths.clear();
	_upload_enabled_flags.clear();
	_cgi_configs.clear();
}

/*********** INITIALISATION DEPUIS CONFIG ***********/

void Server::initServer(const ServerConfig& config) {
	// Copier les données réseau
	_ip = config.host;
	_port = config.port;
	
	// Définir _name avec le premier server_name ou générer un nom par défaut
	if (!config.server_name.empty()) {
		_name = config.server_name[0];  // Utiliser le premier server_name
	} else {
		std::stringstream ss;
		ss << _ip << ":" << _port;
		_name = ss.str();  // Nom par défaut (compatible C++98)
	}
	
	_error_pages = config.error_page;
	_client_max_body_size = config.client_max_body_size;
	
	// Vider les locations existantes
	clearLocations();
	
	// Copier toutes les locations
	for (size_t i = 0; i < config.locations.size(); ++i) {
		const Location& loc = config.locations[i];
		addLocation(loc.path, loc.allow_methods, loc.root, loc.redirect,
				   loc.autoindex, loc.index, loc.upload_path, 
				   loc.upload_enabled, loc.cgi);
	}
	
	std::cout << GREEN << "[INFO] Server initialized: " << _name 
			  << " on " << _ip << ":" << _port << RESET << std::endl;
	std::cout << "  - " << _location_paths.size() << " locations loaded" << std::endl;
}

/*********** GESTION DES LOCATIONS ***********/

int Server::findLocationIndex(const std::string& path) const {
	int best_match_index = -1;
	size_t best_match_length = 0;
	
	for (size_t i = 0; i < _location_paths.size(); ++i) {
		const std::string& location_path = _location_paths[i];
		
		// Vérifier si le chemin commence par le préfixe de la location
		if (path.find(location_path) == 0) {
			if (location_path.length() > best_match_length) {
				best_match_index = (int)i;
				best_match_length = location_path.length();
			}
		}
	}
	
	return best_match_index;
}

std::string Server::getErrorPage(int error_code) const {
	std::map<int, std::string>::const_iterator it = _error_pages.find(error_code);
	if (it != _error_pages.end()) {
		return it->second;
	}
	return "";
}

bool Server::isMethodAllowed(const std::string& path, const std::string& method) const {
	int location_index = findLocationIndex(path);
	
	if (location_index == -1) {
		return false;
	}
	
	const std::vector<std::string>& allowed_methods = _allow_methods_per_location[location_index];
	for (size_t i = 0; i < allowed_methods.size(); ++i) {
		if (allowed_methods[i] == method) {
			return true;
		}
	}
	
	return false;
}

  /********* */	
 /*	STATIC	*/
/********* */

bool	Server::isServerSocket(int fd, std::vector<Server> &server) {
	std::vector<Server>::iterator	it;

	for (it = server.begin(); it != server.end(); ++it) {
		if (it->_socket == fd)
			return true;
	}
	return false;
}

void	Server::closeAllSocket(int epfd, std::vector<Server> &servers, std::vector<Client> &clients) {
	{
		std::vector<Server>::iterator	it;
		for (it = servers.begin(); it != servers.end(); ++it)
			close(it->getSocket());
	}
	{
		std::vector<Client>::iterator	it;
		for (it = clients.begin(); it != clients.end(); ++it) {
			close(it->getSocket());
			delete it->getRequest();
		}
	}
	close(epfd);
}

   /********* */
  /*	DEBUG*/
 /********* */

void Server::printServerInfo() const {
	std::cout << CYAN << "=== Server Information ===" << RESET << std::endl;
	std::cout << "Name: " << _name << std::endl;
	std::cout << "IP: " << _ip << ":" << _port << std::endl;
	std::cout << "Socket FD: " << _socket << std::endl;
	std::cout << "Max Body Size: " << _client_max_body_size << " bytes" << std::endl;
	
	std::cout << "Error Pages (" << _error_pages.size() << "):" << std::endl;
	for (std::map<int, std::string>::const_iterator it = _error_pages.begin();
			it != _error_pages.end(); ++it) 
	{
		std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}
	
	std::cout << "Locations (" << _location_paths.size() << "):" << std::endl;
	for (size_t i = 0; i < _location_paths.size(); ++i)
	{
		std::cout << "  [" << i << "] " << _location_paths[i] << std::endl;
		std::cout << "      Root: " << _roots[i] << std::endl;
		std::cout << "      Methods: ";
		for (size_t j = 0; j < _allow_methods_per_location[i].size(); ++j)
		{
			std::cout << _allow_methods_per_location[i][j];
			if (j < _allow_methods_per_location[i].size() - 1) std::cout << ", ";
		}
		std::cout << std::endl;
		std::cout << "      Autoindex: " << (_autoindex_flags[i] ? "on" : "off") << std::endl;
		std::cout << "      Index: " << _index_files[i] << std::endl;
		if (_upload_enabled_flags[i])
		{
			std::cout << "      Upload: " << _upload_paths[i] << std::endl;
		}
		if (!_cgi_configs[i].empty())
		{
			std::cout << "      CGI: ";
			for (std::map<std::string, std::string>::const_iterator it = _cgi_configs[i].begin();
				 it != _cgi_configs[i].end(); ++it) {
				std::cout << it->first << "->" << it->second << " ";
			}
			std::cout << std::endl;
		}
	}
	std::cout << CYAN << "========================" << RESET << std::endl;
}

// Handle request 

