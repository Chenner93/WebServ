#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>


// Structure représentant une location
struct Location
{
	std::string	path;							// Chemin de la location (ex: /images)
	std::vector<std::string> allow_methods;		// Méthodes HTTP autorisées (GET, POST, DELETE, etc.)
	std::string root;							// Racine du document
	std::string redirect;						// URL de redirection
	bool autoindex;								// Afficher les contenus indexés
	std::string index;							// Fichier d'index
	std::string upload_path;					// Répertoire des uploads
	bool upload_enabled;						// Upload autorisé ou non
	std::map<std::string, std::string> cgi;		// Extension -> path du CGI (.php -> /usr/bin/php-cgi)

	// Constructeur par défaut
	Location()
		: path("/"), autoindex(false), upload_enabled(false) {}
};

// Structure représentant un serveur
struct ServerConfig
{
	std::string host;						// Adresse IP (ex: 127.0.0.1)
	int port;								// Port d'écoute (ex: 8080)
	std::vector<std::string> server_name;	// Noms de serveur optionnels
	std::map<int, std::string> error_page;	// Codes d'erreurs -> Pages d'erreur
	size_t client_max_body_size;			// Taille maximale du corps de la requête en bytes
	std::vector<Location> locations;		// Listes des routes

	// Constructeur par défaut
	ServerConfig()
		: host("127.0.0.1"), port(8080), client_max_body_size(1048576) {} //1Mb par défaut
};

// Classe de configuration principale
class Config
{
	private:

		std::vector<ServerConfig> servers;
		std::string config_file_path;

		// Méthodes privées pour gérer le parsing de la configuration
		void parseServer(const std::string &content, size_t &pos);
		void parseLocation(const std::string &content, size_t &pos, ServerConfig &server);
		std::string parseValue(const std::string &content, size_t &pos);
		std::vector<std::string> parseList(const std::string &content, size_t &pos);
		void skipWhitespace(const std::string &content, size_t &pos);
		void skipComment(const std::string &content, size_t &pos);

		// Validation
		bool validateConfig();
		bool isValidMethod(const std::string &method);
		bool isValidIP(const std::string &ip);
		bool isValidPort(int port);

		// Utils
		std::vector<std::string> split(const std::string &ip, char delimiter);

	public:

		Config();
		Config(const std::string &config_path);
		~Config();

		// Parsing principale
		bool parseConfigFile(const std::string &config_path);

		// Getters
		const std::vector<ServerConfig>& getServers() const;
		ServerConfig *findServer(const std::string &host, int port, const std::string &server_name);

		// Utils
		void printConfig() const; // Debugging
		bool isEmpty() const;
};

#endif // CONFIG_HPP