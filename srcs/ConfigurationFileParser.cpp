#include <Config.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>


static const char* VALID_SERVER_DIRECTIVES[] = 
{
	"listen",
	"server_name",
	"client_max_body_size",
	"error_page",
	"location"
};

static const char* VALID_LOCATION_DIRECTIVES[] = 
{
	"allow_methods",
	"root",
	"return",
	"autoindex",
	"index",
	"upload_dir",
	"cgi_extension"
};

static const size_t VALID_SERVER_DIRECTIVES_SIZE = 
	sizeof(VALID_SERVER_DIRECTIVES) / sizeof(VALID_SERVER_DIRECTIVES[0]);

static const size_t VALID_LOCATION_DIRECTIVES_SIZE = 
	sizeof(VALID_LOCATION_DIRECTIVES) / sizeof(VALID_LOCATION_DIRECTIVES[0]);

Config::Config()
	: config_file_path("./Configuration_Files/DefaultWebserv.conf") {}

Config::Config(const std::string &file_path)
	: config_file_path(file_path) {}

Config::Config(const Config &other)
	: servers(other.servers), config_file_path(other.config_file_path) {}

Config	&Config::operator=(const Config &other)
{
	if (this !=  &other)
	{
		servers = other.servers;
		config_file_path = other.config_file_path;
	}
	return (*this);
}

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

			if (content.substr(pos, 6) == "server")
			{
				pos += 6;
				parseServer(content, pos);
			}
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

void	Config::parseServer(const std::string &content, size_t &pos)
{
	ServerConfig server;

	skipWhitespace(content, pos);

	if (pos >= content.length() || content[pos] != '{')
	{
		throw std::runtime_error("Expected '{' after server");
	}
	pos++;

	int	brace_count = 1;

	while (pos < content.length())
	{
		skipWhitespace(content, pos);
		skipComment(content, pos);

		if (pos >= content.length())
			throw std::runtime_error("Unexpected end of file in server block");

		if (content[pos] == '}')
		{
			brace_count--;
			if (brace_count == 0)
			{
				pos++;
				break;
			}
		}
		else if (content[pos] == '{')
		{
			brace_count++;
		}

		std::string directive;
		while (pos < content.length() && !std::isspace(content[pos]) && content[pos] != ';')
			directive += content[pos++];

		if (directive == "listen")
		{
			skipWhitespace(content, pos);
			std::string listen_value = parseValue(content, pos);

			if (listen_value.empty())
			{
				throw std::runtime_error("Error: 'listen' directive cannot be empty");
			}
		
			size_t colon_pos = listen_value.find(':');
			if (colon_pos != std::string::npos)
			{
				server.host = listen_value.substr(0, colon_pos);

				if (colon_pos + 1 >= listen_value.length())
				{
					throw std::runtime_error("Error: Missing port number after ':' in listen directive");
				}

				std::string port_str = listen_value.substr(colon_pos + 1);

				if (port_str.empty())
				{
					throw std::runtime_error("Error: Port number is empty in listen directive");
				}

				if (!isValidIP(server.host))
				{
					std::ostringstream err;
					err << "Invalid IP address in listen directive: " << server.host;
					throw std::runtime_error(err.str());
				}

				for (size_t i = 0; i < port_str.size(); ++i)
				{
					if (!std::isdigit(port_str[i]))
					{
						std::ostringstream err;
						err << "Invalid port number in listen directive: '" << port_str 
							<< "' (contains non-digit characters)";
						throw std::runtime_error(err.str());
					}
				}

				server.port = std::atoi(port_str.c_str());
			}
			else
			{
				for (size_t i = 0; i < listen_value.size(); ++i)
				{
					if (!std::isdigit(listen_value[i]))
					{
						std::ostringstream err;
						err << "Invalid port number in listen directive: '" << listen_value 
							<< "' (expected format: 'host:port' or 'port')";
						throw std::runtime_error(err.str());
					}
				}
				server.port = std::atoi(listen_value.c_str());
			}
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
			if (!directive.empty() && !isValidServerDirective(directive))
			{
				std::ostringstream err;
				err << "Invalid server directive: " << directive;
				throw std::runtime_error(err.str());
			}
			parseValue(content, pos);
		}
	}

	if (brace_count != 0)
	{
		throw std::runtime_error("Mismatched braces in server block");
	}

	servers.push_back(server);
}

void	Config::parseLocation(const std::string &content, size_t &pos, ServerConfig &server)
{
	Location location;

	skipWhitespace(content, pos);
	location.path = parseValue(content, pos);
	skipWhitespace(content, pos);

	if (pos >= content.length() || content[pos] != '{')
		throw std::runtime_error("Expected '{' after server directive");
	pos++;

	while (pos < content.length())
	{
		skipWhitespace(content, pos);
		skipComment(content, pos);

		if (pos >= content.length())
			throw std::runtime_error("Unexpected end of file in Location block");

		if (content[pos] == '}')
		{
			pos++;
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
			location.upload_enabled = 1;
		}
		else if (directive == "cgi_extension")
		{
			skipWhitespace(content, pos);
			std::vector<std::string> cgi_config = parseList(content, pos);
			if (cgi_config.size() >= 2)
			{
				// if (access(cgi_config[1].c_str(), X_OK) != 0)
				// {
				// 	std::ostringstream err;
				// 	err << "CGI interpreter not found or not executable: " << cgi_config[1];
				// 	throw std::runtime_error(err.str());
				// }
				location.cgi[cgi_config[0]] = cgi_config[1];
			}
		}
		else
		{
			if (!directive.empty() && !isValidLocationDirective(directive))
			{
				std::ostringstream err;
				err << "Unknown directive '" << directive << "' in location block. "
				<< "Valid directives are: allow_methods, root, return, autoindex, index, upload_dir, cgi_extension";
			throw std::runtime_error(err.str());
			}
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

	skipWhitespace(content, pos);
	if (pos < content.length() && content[pos] == ';')
		pos++;

	return value;
}

std::vector<std::string>	Config::parseList(const std::string& content, size_t& pos)
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

	skipWhitespace(content, pos);
	if (pos < content.length() && content[pos] == ';')
		pos++;

	return list;
}

bool	Config::validateConfig()
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
bool	Config::isValidMethod(const std::string& method)
{
	return (method == "GET" || method == "POST" || method == "DELETE" || method == "HEAD");
}

std::vector<std::string>	Config::split(const std::string &ip, char delimiter)
{
	std::vector<std::string> result;
	std::string current;

	for (size_t i = 0; i < ip.length(); i++)
	{
		if (ip[i] == delimiter)
		{
			if (!current.empty())
			{
				result.push_back(current);
				current.clear();
			}
		}
		else
		{
			current += ip[i];
		}
	}
	if (!current.empty())
	{
		result.push_back(current);
	}
	return result;
}

bool	Config::isValidIP(const std::string& ip) {
	// Cas spéciaux
	if (ip == "0.0.0.0" || ip == "localhost") {
		return (true);
	}

	std::vector<std::string> octets = split(ip, '.');

	// Doit avoir exactement 4 octets
	if (octets.size() != 4) {
		std::cerr << "Error: IP address must have 4 octets, got " 
				  << octets.size() << std::endl;
		return (false);
	}

	for (size_t i = 0; i < octets.size(); ++i) {
		const std::string& octet = octets[i];

		if (octet.empty()) {
			std::cerr << "Error: Empty octet in IP address" << std::endl;
			return (false);
		}

		for (size_t j = 0; j < octet.length(); ++j) {
			if (!std::isdigit(octet[j])) {
				std::cerr << "Error: Invalid character '" << octet[j] 
						  << "' in IP address octet" << std::endl;
				return (false);
			}
		}

		if (octet.length() > 1 && octet[0] == '0') {
			std::cerr << "Error: Leading zeros not allowed in IP octet: " 
					  << octet << std::endl;
			return (false);
		}

		long value = std::atol(octet.c_str());

		if (value < 0 || value > 255) {
			std::cerr << "Error: IP octet " << octet << " is out of range [0-255]" 
					  << std::endl;
			return (false);
		}
	}

	return (true);
}

bool	Config::isValidPort(int port)
{
	if (port <= 0)
	{
		std::ostringstream err;
		err << "Port number must be positive (got " << port << ")";
		throw std::runtime_error(err.str());
	}
	if (port > 65535)
	{
		std::ostringstream err;
		err << "Port number must be less than 65536 (got " << port << ")";
		throw std::runtime_error(err.str());
	}
	if (port < 1024)
	{
		std::cerr << "Warning: Using a privileged port (<1024) may require elevated permissions." << std::endl;
	}
	return (true);
}

bool	Config::isValidServerDirective(const std::string& directive)
{
	size_t count = sizeof(VALID_SERVER_DIRECTIVES_SIZE);
	for (size_t i = 0; i < count; ++i)
	{
		if (directive == VALID_SERVER_DIRECTIVES[i])
			return (true);
	}
	return (false);
}

bool	Config::isValidLocationDirective(const std::string& directive)
{
	size_t count = sizeof(VALID_LOCATION_DIRECTIVES_SIZE);
	for (size_t i = 0; i < count; ++i)
	{
		if (directive == VALID_LOCATION_DIRECTIVES[i])
			return (true);
	}
	return (false);
}

const	std::vector<ServerConfig>& Config::getServers() const
{
	return (servers);
}

ServerConfig	*Config::findServer(const std::string &host, int port, const std::string &server_name)
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

void	Config::skipWhitespace(const std::string& content, size_t& pos)
{
	while (pos < content.length() && std::isspace(content[pos]))
		pos++;
}

void	Config::skipComment(const std::string& content, size_t& pos)
{
	if (pos < content.length() && content[pos] == '#') {
		while (pos < content.length() && content[pos] != '\n')
			pos++;
		if (pos < content.length())
			pos++;
		skipWhitespace(content, pos);
	}
}

void	Config::printConfig() const
{
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

bool	Config::isEmpty() const
{
	return (servers.empty());
}
