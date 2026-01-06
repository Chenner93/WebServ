/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_if_CGI.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckenaip <ckenaip@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/29 14:16:50 by kahoumou          #+#    #+#             */
/*   Updated: 2026/01/06 16:35:19 by ckenaip          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Request/Request.hpp>

std::string	Request::getPath() {
	return this->path;
}


bool Request::check_path_cgi(const std::string &path, const std::string &suffix) 
{
	if (path.length() < suffix.length())
		return false;
	return path.compare(path.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool	Request::isPython() {
	return check_path_cgi(this->getPath(), ".py");
}

bool	Request::isPhp() {
	return check_path_cgi(this->getPath(), ".php");
}

bool Request::Python_Or_Php() 
{
    return check_path_cgi(this->getPath(), ".py") || check_path_cgi(this->getPath(), ".php");
}

void Request::extract_path_after_sign(const std::string &path, char sign)
{
    std::string::size_type pos = path.find(sign);

    if (pos == std::string::npos)
    {
        path_after_sign = "";
        return;
    }
    path_after_sign = path.substr(pos + 1);
}

const std::string Request::getExtension(std::string path) {

	size_t	findLast = 0;
	findLast = path.rfind(".");
	if (findLast == std::string::npos)
		return "";

	return path.substr(findLast);
}


void	Request::printHeader() {
	std::map<std::string, std::string>::iterator it;
	for (it = headers.begin(); it != headers.end(); ++it) {
	    std::cerr << it->first << ": " << it->second << std::endl;
	}
}

std::string	Request::getHeader(std::string toFind) {

	std::map<std::string, std::string>::iterator	it;

	it = this->headers.find(toFind);
	if (it != this->headers.end()) {
		return it->second;
	}
	
	return "";
}
