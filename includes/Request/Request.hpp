/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 13:59:50 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/15 15:40:54 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP
#include<string>
#include<map>
#include<sstream>
#include<vector>
#include <stdexcept>
#include<iomanip>
#include <netinet/in.h>
#include"../Client.hpp"
#include"Utils_parsing.hpp"
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
class Client;

class Request
{
    private:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string>headers;
    std::string body;
    std::map<std::string, std::string> url_params;
    
    public:
    Request(const std::string& line_request);
    Request &operator = (const Request& other);
    std::vector<std::string> parse_all_lines(const std::string& lines); 
    void parse_one_line(const std::vector<std::string>& line);
    const std::string& getMethod() const;
    const std::string& getPath() const;
    const std::string& getVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    void parseRequest(const std::string&  line_request);
    bool split_http_head_body(const std::string& raw, std::string& head, std::string& body);
    const std::map<std::string, std::string>& getUrlparams()  const;
    bool hasUrlParam(const  std::string& key) const;
    std::string getUrlParam(const std::string& key) const;
    void parse_url();
    void print_request(const Request& req);
   static void handleClientRequest(Client &client);
};

#endif