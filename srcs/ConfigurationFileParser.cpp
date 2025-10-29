#include <Config.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

Config::Config()
	: config_file_path("./Configuration_Files/DefaultWebserv.conf") {}

Config::Config(const std::string &file_path)
	: config_file_path(file_path) {}

Config::~Config() {}

bool	Config::parseConfigFile(const std::string &config_path)
{
	std::string path = config_path.empty() ? config_file_path : config_path;

	std::ifstream file(path.c_str());
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open config file: " << path << std::endl;
		return false;
	}

	// Lire tout le fichier en mémoire
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();

	size_t pos = 0;
	try
	{
		while (pos < content.length())
		{
			skipWhitespace(content, pos);
			skipComment(content, pos);

			if (pos >= content.length())
				break;

			// Détecter et parser un bloc server
			if (content.substr(pos, 6) == "server")
			{
				pos += 6; // Move past the "server" keyword
				parseServer(content, pos);
			}
			// Ignorer les autres lignes
			else
			{
				while (pos < content.length() && content[pos] != '\n')
					pos++;
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error while parsing config file: " << e.what() << std::endl;
		return (false);
	}

	return (validateConfig());
}

void Config::parseServer(const std::string &content, size_t &pos)
{
	ServerConfig server;

	skipWhitespace(content, pos);

	// Attendre une accolade ouvrante
	if (pos >= content.length() || content[pos] != '{')
	{
		throw std::runtime_error("Expected '{' after server");
	}
	pos++;

	while (pos < content.length())
	{
		skipWhitespace(content, pos);
		skipComment(content, pos);

		if (pos >= content.length())
			throw std::runtime_error("Unexpected end of file in server block");

		if (content[pos] == '}')
		{
			pos++;
			break;
		}

		// Parser les directives du serveur
		std::string directive;
		while (pos < content.length() && !std::isspace(content[pos]) && content[pos] != ';')
			directive += content[pos++];

		if (directive == "listen")
		{
			skipWhitespace(content, pos);
			std::string listen_value = parseValue(content, pos);

			// Parser host:port ou juste port
			size_t colon_pos = listen_value.find(':');
			if (colon_pos != std::string::npos)
			{
				server.host = listen_value.substr(0, colon_pos);
				server.port = std::atoi(listen_value.substr(colon_pos + 1).c_str());
			}
			else
				server.port = std::atoi(listen_value.c_str());
		}
		else if (directive == "server_name")
		{
			skipWhitespace(content, pos);
			server.server_name = parseList(content, pos);
		}
		else if (directive == "client_max_body_size")
		{
			skipWhitespace(content, pos);
			std::string size_str = parseValue(content, pos);
			server.client_max_body_size = std::atoi(size_str.c_str());

			// Gérer les suffixes (K, M, G)
			if (!size_str.empty())
			{
				char suffix = std::toupper(size_str[size_str.length() - 1]);
				if (suffix == 'K') server.client_max_body_size *= 1024;
				else if (suffix == 'M') server.client_max_body_size *= 1024 * 1024;
				else if (suffix == 'G') server.client_max_body_size *= 1024 * 1024 * 1024;
			}
		}
		else if (directive == "error_page")
		{
			skipWhitespace(content, pos);
			std::vector<std::string> error_config = parseList(content, pos);
			if (error_config.size() >= 2) {
				int error_code = std::atoi(error_config[0].c_str());
				server.error_page[error_code] = error_config[1];
			}
		}
		else if (directive == "location")
		{
			parseLocation(content, pos, server);
		}
		else
		{
			// Ignorer les directives inconnues
			parseValue(content, pos);
		}
	}

	servers.push_back(server);
}

void Config::parseLocation(const std::string &content, size_t &pos, ServerConfig &server)
{
	Location location;

	skipWhitespace(content, pos);
	location.path = parseValue(content, pos);
	skipWhitespace(content, pos);

	if (pos >= content.length() || content[pos] != '{')
		throw std::runtime_error("Expected '{' after server directive");
	pos++; // Skip '{'

	while (pos < content.length())
	{
		skipWhitespace(content, pos);
		skipComment(content, pos);

		if (pos >= content.length())
			throw std::runtime_error("Unexpected end of file in Location block");

		if (content[pos] == '}')
		{
			pos++; // Skip '}'
			break;
		}

		std::string directive;
		while (pos < content.length() && !std::isspace(content[pos]) && content[pos] != ';')
			directive += content[pos++];
		
		if (directive == "allow_methods")
		{
			skipWhitespace(content, pos);
			location.allow_methods = parseList(content, pos);
		}
		else if (directive == "root")
		{
			skipWhitespace(content, pos);
			location.root = parseValue(content, pos);
		}
		else if (directive == "return")
		{
			skipWhitespace(content, pos);
			location.redirect = parseValue(content, pos);
		}
		else if (directive == "autoindex")
		{
			skipWhitespace(content, pos);
			std::string value = parseValue(content, pos);
			location.autoindex = (value == "on");
		}
		else if (directive == "index")
		{
			skipWhitespace(content, pos);
			location.index = parseValue(content, pos);
		}
		else if (directive == "upload_dir")
		{
			skipWhitespace(content, pos);
			location.upload_path = parseValue(content, pos);
			location.upload_enabled = !location.upload_path.empty();
		}
		else if (directive == "cgi_extension")
		{
			skipWhitespace(content, pos);
			std::vector<std::string> cgi_config = parseList(content, pos);
			if (cgi_config.size() >= 2)
			{
				location.cgi[cgi_config[0]] = cgi_config[1];
			}
		}
		else
		{
			parseValue(content, pos);
		}
	}
	server.locations.push_back(location);
}

std::string Config::parseValue(const std::string& content, size_t& pos)
{
	std::string value;

	skipWhitespace(content, pos);

	while (pos < content.length() && content[pos] != ';' && content[pos] != '\n' && !std::isspace(content[pos]))
	{
		value += content[pos++];
	}

	// Ignorer le point-virgule optionnel
	skipWhitespace(content, pos);
	if (pos < content.length() && content[pos] == ';')
		pos++;

	return value;
}

std::vector<std::string> Config::parseList(const std::string& content, size_t& pos)
{
	std::vector<std::string> list;
	std::string current_item;

	skipWhitespace(content, pos);

	while (pos < content.length() && content[pos] != ';' && content[pos] != '\n')
	{
		if (std::isspace(content[pos]))
		{
			if (!current_item.empty())
			{
				list.push_back(current_item);
				current_item.clear();
			}
			skipWhitespace(content, pos);
		}
		else
		{
			current_item += content[pos++];
		}
	}

	if (!current_item.empty())
	{
		list.push_back(current_item);
	}

	// Ignorer le point-virgule optionnel
	skipWhitespace(content, pos);
	if (pos < content.length() && content[pos] == ';')
		pos++;

	return list;
}

bool Config::validateConfig()
{
	if (servers.empty())
	{
		std::cerr << "Error: No server blocks defined in configuration." << std::endl;
		return false;
	}

	for (size_t i = 0; i < servers.size(); i++)
	{
		const ServerConfig &server = servers[i];
		if (!isValidPort(server.port))
		{
			std::cerr << "Error: Invalid port number: " << server.port << std::endl;
			return false;
		}

		for (size_t j = 0; j < server.locations.size(); j++)
		{
			const Location &location = server.locations[j];
			for (size_t k = 0; k < location.allow_methods.size(); k++)
			{
				const std::string &method = location.allow_methods[k];
				if (!isValidMethod(method))
				{
					std::cerr << "Error: Invalid HTTP method in location " << location.path << ": " << method << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}
bool Config::isValidMethod(const std::string& method)
{
	return (method == "GET" || method == "POST" || method == "DELETE" || method == "HEAD");
}

bool Config::isValidPort(int port)
{
	return (port > 0 && port <= 65535);
}

const std::vector<ServerConfig>& Config::getServers() const
{
	return servers;
}

ServerConfig *Config::findServer(const std::string &host, int port, const std::string &server_name)
{
	if (!server_name.empty())
	{
		for (size_t i = 0; i < servers.size(); i++)
		{
			ServerConfig &server = servers[i];
			if (server.host == host && server.port == port)
			{
				for (size_t j = 0; j < server.server_name.size(); j++)
				{
					const std::string &name = server.server_name[j];
					if (name == server_name)
						return (&server);
				}
			}
		}
	}

	for (size_t i = 0; i < servers.size(); i++)
	{
		ServerConfig &server = servers[i];
		if (server.host == host && server.port == port)
			return (&server);
	}
	return (NULL);
}

void Config::skipWhitespace(const std::string& content, size_t& pos)
{
	while (pos < content.length() && std::isspace(content[pos]))
		pos++;
}

void Config::skipComment(const std::string& content, size_t& pos) {
	if (pos < content.length() && content[pos] == '#') {
		while (pos < content.length() && content[pos] != '\n')
			pos++;
		if (pos < content.length())
			pos++; // Skip the newline
		skipWhitespace(content, pos);
	}
}

void Config::printConfig() const {
	for (size_t i = 0; i < servers.size(); ++i) {
		const ServerConfig& server = servers[i];
		std::cout << "Server " << i + 1 << ":" << std::endl;
		std::cout << "	Host: " << server.host << std::endl;
		std::cout << "	Port: " << server.port << std::endl;
		std::cout << "	Max body size: " << server.client_max_body_size << std::endl;

		if (!server.server_name.empty()) {
			std::cout << "	Server names: ";
			for (size_t j = 0; j < server.server_name.size(); ++j) {
				std::cout << server.server_name[j];
				if (j < server.server_name.size() - 1) std::cout << ", ";
			}
			std::cout << std::endl;
		}

		std::cout << "	Locations:" << std::endl;
		for (size_t j = 0; j < server.locations.size(); ++j) {
			const Location& location = server.locations[j];
			std::cout << "		" << location.path << std::endl;
			std::cout << "			Root: " << location.root << std::endl;
		}
		std::cout << std::endl;
	}
}

bool Config::isEmpty() const
{
	return (servers.empty());
}
