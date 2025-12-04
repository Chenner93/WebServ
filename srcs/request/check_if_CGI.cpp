/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_if_CGI.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckenaip <ckenaip@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/29 14:16:50 by kahoumou          #+#    #+#             */
/*   Updated: 2025/12/04 16:48:05 by ckenaip          ###   ########.fr       */
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