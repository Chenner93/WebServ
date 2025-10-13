/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckenaip <ckenaip@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 14:48:50 by kahoumou          #+#    #+#             */
/*   Updated: 2025/10/13 15:37:04 by ckenaip          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Request/Request.hpp"
#include "../../includes/Request/Response.hpp"

Request::Request(const std::string& rawRequest, Server *server) 
{
	_server = server;
    parseRequest(rawRequest);
}

Request& Request::operator=(const Request& other)
{
    if (this != &other)
    {
        method  = other.method;
        path    = other.path;
        version = other.version;
        headers = other.headers;
        body    = other.body;
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

    method  = parts[0];
    path    = parts[1];
    version = parts[2];
}


std::vector<std::string> Request::parse_all_lines(const std::string& head)
{
    
    return utils_parsing::split_on_substr(head, "\r\n", /*keep_empty=*/false);
}




// void Request::parseRequest(const std::string& raw)
// {
//     if (raw.empty())
//         throw std::runtime_error("empty request");

//     std::string head;
//     if (!split_http_head_body(raw, head, raw_body))
//         throw std::runtime_error("incomplete HTTP headers");

//     std::vector<std::string> lines = Request::parse_all_lines(head);
//     if (lines.empty())
//         throw std::runtime_error("invalid request: empty head");

 
//     Request::parse_one_line(lines);
//     Request::parse_url();
//     headers.clear();
    
    
    
//     for (std::size_t i = 1; i < lines.size(); ++i)
//     {
//         const std::string& line = lines[i];
//         if (line.empty()) continue;

        
//         std::size_t pos = line.find(':');
//         if (pos == std::string::npos)
//             continue; 

//         std::string key   = utils_parsing::trim(line.substr(0, pos));
//         std::string value = utils_parsing::trim(line.substr(pos + 1));

//         if (!key.empty())
//             headers[utils_parsing::to_lower(key)] = value;
//     }
//      body = raw_body;
//    if (headers.count("transfer-encoding") &&
//     utils_parsing::to_lower(headers["transfer-encoding"]) == "chunked")
// {
//     std::cout << GREEN << "raw_body is " << raw_body << RESET << std::endl;
//     std::cout << YELLOW << "pass in  cond if parseChunkedBody" << RESET << std::endl;
//     this->body = parseChunkedBody(raw_body);
// }
// else
// {
//     this->body = raw_body;
// }

// }


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

        std::string key   = utils_parsing::trim(line.substr(0, pos));
        std::string value = utils_parsing::trim(line.substr(pos + 1));

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

    std::cout << YELLOW << "Body:" << RESET << "\n"
              << req.getBody() << "\n";
    std::cout << CYAN << "------------------------" << RESET << "\n\n";
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

void Request::handleClientRequest(Client &client) 
{
    char buffer[4096];
    ssize_t bytes_received = recv(client.getSocket(), buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        return;
    }
    buffer[bytes_received] = '\0';
    std::string rawRequest(buffer);

    try {
        Request request(rawRequest, client.getPtrServer());
        Response response;
        request.parse_url();
        request.print_request(request);
        response.Methodes(request);
    } catch (const std::exception &e) {
        std::cerr << "Bad Request: " << e.what() << std::endl;
    }
}

 



// int main()
// {
//     std::string raw1 =
//         "GET /test HTTP/1.1\r\n"
//         "Host: example.com\r\n"
//         "X-Custom: line1\r\n"
//         "  line2-continued\r\n"
//         "\r\n";

   
//     std::string raw2 =
//         "POST /upload HTTP/1.1\r\n"
//         "Host: example.com\r\n"
//         "Transfer-Encoding: chunked\r\n"
//         "\r\n"
//         "5\r\nHello\r\n"
//         "6\r\n World\r\n"
//         "0\r\n\r\n";

  
//     std::string raw3 =
//         "POST /short HTTP/1.1\r\n"
//         "Host: example.com\r\n"
//         "Content-Type: text/plain\r\n"
//         "Content-Length: 11\r\n"
//         "\r\n"
//         "Hello";

//     try {
//         std::cout << BLUE << "[TEST 1: Header multiline]" << RESET << "\n";
//         Request r1(raw1);
//         print_request(r1);
//     } catch (std::exception& e) {
//         std::cout << RED << "Test 1 failed: " << e.what() << RESET << "\n";
//     }

//     try {
//         std::cout << BLUE << "[TEST 2: Chunked Transfer-Encoding]" << RESET << "\n";
//         Request r2(raw2);
//         print_request(r2);
//     } catch (std::exception& e) {
//         std::cout << RED << "Test 2 failed: " << e.what() << RESET << "\n";
//     }

//     try {
//         std::cout << BLUE << "[TEST 3: Content-Length incohérent]" << RESET << "\n";
//         Request r3(raw3);
//         print_request(r3);
//     } catch (std::exception& e) {
//         std::cout << RED << "Test 3 failed: " << e.what() << RESET << "\n";
//     }
//         std::string raw =
//         "GET /search?q=openai&lang=fr&debug=true HTTP/1.1\r\n"
//         "Host: localhost:8080\r\n"
//         "User-Agent: curl/7.81.0\r\n"
//         "\r\n";

//     Request req(raw);         // ton constructeur appelle parseRequest → qui appelle parse_url
//     printUrlParams(req);      // test d'affichage des paramètres

//     // ----------- TESTS decode_url ------------
//     std::cout << GREEN << "\n[TESTS: decode_url]\n" << RESET;

//     std::vector<std::string> inputs = {
//     "Jean%20Dupont",
//     "email%3Auser%40test.com",
//     "path%2Fto%2Ffile",
//     "value+with+plus",
//     "normaltext",
//     "%2G",     // invalide
//     "%",       // invalide
//     "end%",    // invalide
//     "mixed%20text+with%2Fencoded+stuff"
// };

// for (std::size_t i = 0; i < inputs.size(); ++i)
// {
//     std::string decoded = utils_parsing::decode_url(inputs[i]);
//     std::cout << CYAN << "Test " << i + 1 << RESET << " | ";
//     std::cout << "Input: \"" << inputs[i] << "\" → ";
//     std::cout << "Decoded: \"" << decoded << "\"\n";
// }
// std::string test = "nom=Jean%20Dupont&ville=St%20Ouen&erreur=%GZ&incomplet=%5";
// std::vector<std::string> pairs = utils_parsing::split_on_substr(test, "&", false);

// for (size_t i = 0; i < pairs.size(); ++i)
// {
// 	std::size_t eq = pairs[i].find('=');
// 	if (eq == std::string::npos)
// 		continue;

// 	std::string key = utils_parsing::decode_url(pairs[i].substr(0, eq));
// 	std::string value = utils_parsing::decode_url(pairs[i].substr(eq + 1));

// 	std::cout << key << " = " << value << "\n";
// }


//     return 0;
// }



