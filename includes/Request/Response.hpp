/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:45:45 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/20 12:15:18 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include<string>
#include<sstream>
#include<fstream>
#include<unistd.h>
#include<sys/stat.h>
#include<iostream>
#include<fcntl.h>
#include <map>

class Response
{
    public:
    //  Response();
    //  ~Response();

    //  static void Methodes(const Request &req, int client_fd);
     std::string Methodes(const Request &request);
    //  static  handleGet(const  Request& request, int client_fd);
    std::string handleGet(const Request &request);
     std::string  handlePost(const  Request& req);
     std::string  handleDelete(const  Request& req);
     std::string sendError(int code, const std::string& msg);

};

#endif
