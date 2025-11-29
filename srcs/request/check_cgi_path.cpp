/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_cgi_path.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/29 14:16:50 by kahoumou          #+#    #+#             */
/*   Updated: 2025/11/29 14:17:09 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */




#include "../../includes/Request/Response.hpp"
#include "../../includes/Server.hpp"


bool Response::check_path_cgi(const std::string &path, const std::string &suffix) 
{
    if (path.length() < suffix.length())
        return false;
    return path.compare(path.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool Response::Python_Or_Php(const std::string &path) 
{
    return check_path_cgi(path, ".py") || check_path_cgi(path, ".php");
}