/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thbasse <thbasse@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 14:48:50 by kahoumou          #+#    #+#             */
/*   Updated: 2025/11/20 15:17:05 by thbasse          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Request/Request.hpp"
#include "../../includes/Request/Response.hpp"
#include "../../includes/Server.hpp"

Request::Request(const std::string& rawRequest, Server *server) 
{
	_server = server;
	parseRequest(rawRequest);
}

Request& Request::operator=(const Request& other)
{
	if (this != &other)
	{
		method	= other.method;
		path	= other.path;
		version	= other.version;
		headers	= other.headers;
		body	= other.body;
	}
	return *this;
}

bool Request::split_http_head_body(const std::string& raw,
									std::string& head,
									std::string& body_part)
{
	std::cout << GREEN << "[DEBUG] ENTREE parseRequest() → RAW COMPLET ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓" << RESET << std::endl;
	std::cout << raw << std::endl;
	std::cout << GREEN << "[DEBUG] FIN RAW" << RESET << std::endl;

	const std::string sep = "\r\n\r\n";
	std::size_t p = raw.find(sep);
	if (p == std::string::npos) return false;
	head = raw.substr(0, p);
	body_part = raw.substr(p + sep.size()); 
	return true;
}

void Request::parse_one_line(const std::vector<std::string>& line)
{
	if (line.empty())
		throw std::runtime_error("invalid request: missing request-line");

	std::vector<std::string> parts = utils_parsing::ft_split(line[0], ' ');
	if (parts.size() != 3)
		throw std::runtime_error("invalid request line");

	method	= parts[0];
	std::cout<<RED<<method<<std::endl;
	path	= parts[1];
	version	= parts[2];
}

std::vector<std::string> Request::parse_all_lines(const std::string& head)
{
	return utils_parsing::split_on_substr(head, "\r\n", /*keep_empty=*/false);
}

void Request::parseRequest(const std::string& raw)
{
	if (raw.empty())
		throw std::runtime_error("empty request");

	std::string head;
	if (!split_http_head_body(raw, head, raw_body))
		throw std::runtime_error("incomplete HTTP headers");

	// Analyse ligne par ligne
	std::vector<std::string> lines = Request::parse_all_lines(head);
	if (lines.empty())
		throw std::runtime_error("invalid request: empty head");

	Request::parse_one_line(lines);
	Request::parse_url();

	// Parser les headers
	headers.clear();
	for (std::size_t i = 1; i < lines.size(); ++i)
	{
		const std::string& line = lines[i];
		if (line.empty()) continue;

		std::size_t pos = line.find(':');
		if (pos == std::string::npos)
			continue;

		std::string key		= utils_parsing::trim(line.substr(0, pos));
		std::string value	= utils_parsing::trim(line.substr(pos + 1));

		if (!key.empty())
			headers[utils_parsing::to_lower(key)] = value;
	}

	// Gestion du Transfer-Encoding: chunked
	if (headers.count("transfer-encoding") &&
		utils_parsing::to_lower(headers["transfer-encoding"]) == "chunked")
	{
		std::cout << GREEN << "[DEBUG] raw_body is:\n[" << raw_body << "]" << RESET << std::endl;
		std::cout << YELLOW << "[DEBUG] → Transfer-Encoding is chunked" << RESET << std::endl;

		this->body = parseChunkedBody(raw_body);
	}
	else
	{ 
		std::cout << YELLOW << "[DEBUG] → Transfer-Encoding is NOT chunked" << RESET << std::endl;
		this->body = raw_body;
	}
}

const std::string& Request::getMethod() const
{
	return method;
}

const std::string& Request::getPath() const
{
	return path;
}
const std::string& Request::getVersion() const
{
	return version;
}
const std::map<std::string, std::string>& Request::getHeaders() const
{
	return headers;
}
const std::string& Request::getBody() const
{
	return body;
}

void Request::print_request(const Request& req)
{
	std::cout << CYAN << "---- Parsed Request ----" << RESET << "\n";
	std::cout << GREEN << "Method:  " << RESET << req.getMethod() << "\n";
	std::cout << GREEN << "Path:    " << RESET << req.getPath() << "\n";
	std::cout << GREEN << "Version: " << RESET << req.getVersion() << "\n";

	std::cout << YELLOW << "Headers:" << RESET << "\n";
	for (std::map<std::string,std::string>::const_iterator it = req.getHeaders().begin();
		 it != req.getHeaders().end(); ++it)
	{
		std::cout << "  " << MAGENTA << it->first << RESET
				  << " -> " << BLUE << it->second << RESET << "\n";
	}

	std::cout	<< YELLOW << "Body:" << RESET << "\n"
				<< req.getBody() << "\n";
	std::cout	<< CYAN << "------------------------" << RESET << "\n\n";
}

void printUrlParams(const Request& req)
{
	const std::map<std::string, std::string>& params = req.getUrlparams();

	std::cout << "\n🔎 URL Parameters:\n";
	if (params.empty()) {
		std::cout << "  (aucun paramètre)\n";
		return;
	}

	for (std::map<std::string, std::string>::const_iterator it = params.begin(); it != params.end(); ++it)
	{
		std::cout << "  " << it->first << " = " << it->second << "\n";
	}
}

void Request::printFormDataParts(const std::vector<FormDataPart>& parts)
{
	std::cout << CYAN << "---- Parsed Multipart Form Data ----" << RESET << std::endl;

	if (parts.empty()) {
		std::cout << RED << "Aucune partie trouvée dans la requête." << RESET << std::endl;
		return;
	}

	for (size_t i = 0; i < parts.size(); ++i)
	{
		std::cout << YELLOW << "[Part " << i << "]" << RESET << std::endl;

		std::cout	<< GREEN << "  name: " << RESET << parts[i].name << std::endl;
		std::cout	<< GREEN << "  filename: " << RESET
					<< (parts[i].filename.empty() ? "(vide)" : parts[i].filename) << std::endl;
		std::cout	<< "[DEBUG] Content length: " << parts[i].content.size() << " bytes" << std::endl;

		std::cout	<< GREEN << "  contentType: " << RESET
					<< (parts[i].contentType.empty() ? "(non spécifié)" : parts[i].contentType) << std::endl;
		std::cout	<< BLUE << "  content (taille = " << parts[i].content.size() << " octets):" << RESET << std::endl;

		// Afficher les 200 premiers caractères du contenu max pour éviter le flood
		std::string preview = parts[i].content.substr(0, 200);
		std::cout << BLUE << "  content (taille = " << parts[i].content.size() << " octets):" << RESET << std::endl;
		std::cout << preview;
		if (parts[i].content.size() > 200)
			std::cout << "..." << std::endl;
		else
			std::cout << std::endl;

		std::cout << CYAN << "-----------------------------------" << RESET << std::endl;
	}
	std::cout << std::endl;
}

void Request::handleClientRequest(Client &client) 
{
	char buffer[4096];
	ssize_t bytes_received = recv(client.getSocket(), buffer, sizeof(buffer) - 1, 0);
	if (bytes_received <= 0)
		return;
	buffer[bytes_received] = '\0';
	std::string rawRequest(buffer);

	try {
		Request request(rawRequest, client.getPtrServer());
		Response response;
		Server serv;
		request.parse_url();
		request.print_request(request);
		response.Methodes(request, serv);
	} catch (const std::exception &e) {
		std::cerr << "Bad Request: " << e.what() << std::endl;
	}
}
