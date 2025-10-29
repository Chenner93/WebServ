/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:45:45 by kahoumou          #+#    #+#             */
/*   Updated: 2025/10/29 15:27:43 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Request.hpp"
# include <fcntl.h>
# include <fstream>
# include <iostream>
# include <map>
# include <sstream>
# include <string>
# include <sys/stat.h>
# include <unistd.h>

// class Response
// {
//   public:
// 	//  Response();
// 	//  ~Response();

// 	//  static void Methodes(const Request &req, int client_fd);
// 	//  std::string Methodes(const Request &request);
// 	std::string Methodes(const Request &request, const Server &server);
// 	//  static  handleGet(const  Request& request, int client_fd);
// 	// std::string handleGet(const Request &request);
// 	//  std::string  handlePost(const  Request& req);
// 	//  std::string  handleDelete(const  Request& req);
// 	std::string handleGet(const Request &request, const Server &server);
// 	std::string handlePost(const Request &request, const Server &server);
// 	std::string handleDelete(const Request &request, const Server &server);

// 	std::string sendError(int code, const std::string &msg);
// 	// void saveFormDataToDisk(const Request::FormDataPart &part);
//     void saveFormDataToDisk(const Request::FormDataPart& part,
//                                   const std::string& upload_dir)
// 	std::string sendErrorAsString(int code, const std::string &message);
// };

class Response {

public:
	// Response();
	// ~Response();

	// Méthode principale appelée depuis main / Client
	std::string Methodes(const Request &request, const Server &server);

	// Méthodes HTTP
	std::string handleGet(const Request &request, const Server &server);
	std::string handlePost(const Request &request, const Server &server);
	std::string handleDelete(const Request &request, const Server &server);
	std::string handleHead(const Request &request, const Server &server);

	// Gestion d'erreur HTTP
	std::string sendError(int code, const std::string& msg);

	// Sauvegarde multipart (upload)
	void saveFormDataToDisk(const Request::FormDataPart& part,
							const std::string& upload_dir);

	// Type MIME
	std::string getContentType(const std::string& path);
};

#endif
