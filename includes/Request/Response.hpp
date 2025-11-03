/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thbasse <thbasse@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:45:45 by kahoumou          #+#    #+#             */
/*   Updated: 2025/11/03 14:59:39 by thbasse          ###   ########.fr       */
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

	// Génération d'autoindex
	std::string generateAutoindex(const std::string& dirPath, const std::string& requestPath);

	// Sauvegarde multipart (upload)
	void saveFormDataToDisk(const Request::FormDataPart& part,
							const std::string& upload_dir);

	// Type MIME
	std::string getContentType(const std::string& path);
};

std::string strip_location_prefix(const std::string& path, const std::string& loc_prefix);
std::string join_path(const std::string& a, const std::string& b);

#endif
