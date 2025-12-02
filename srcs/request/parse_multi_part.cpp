/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_multi_part.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckenaip <ckenaip@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/20 12:08:53 by kahoumou          #+#    #+#             */
/*   Updated: 2025/11/28 18:40:51 by ckenaip          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../../includes/Request/Request.hpp"


std::string  Request::ParseBoundary(const std::map<std::string, std::string>& headers)
{
    std::map<std::string, std::string>::const_iterator it = headers.find("content-type");
    if(it == headers.end())
        return("");
    const std::string& contentType = it->second;
    size_t pos = contentType.find("boundary=");
    if(pos == std::string::npos)
        return("");
    return("--" + contentType.substr(pos + 9));
}

std::map<std::string, std::string> Request::parseContentDisposition(const std::string& line)
{
    std::map<std::string, std::string> result;

    
    std::string::size_type start = 0;
    std::string::size_type end = 0;

    while ((start = line.find(";", start)) != std::string::npos)
    {
        ++start; 
        while (start < line.size() && line[start] == ' ')
            ++start;

        end = line.find("=", start);
        if (end == std::string::npos)
            break;

        std::string key = line.substr(start, end - start);
        std::string value;

        if (line[end + 1] == '"')
        {
            std::string::size_type val_start = end + 2;
            std::string::size_type val_end = line.find('"', val_start);
            if (val_end == std::string::npos)
                break;
            value = line.substr(val_start, val_end - val_start);
            start = val_end + 1;
        }
        else
        {
            std::string::size_type val_end = line.find(";", end + 1);
            value = line.substr(end + 1, val_end - (end + 1));
            start = val_end;
        }

        result[key] = value;
    }

    return result;
}


// std::vector<Request::FormDataPart> Request::parseMultipartFormData(const std::string& body, const std::string& boundary)
// {
// 	std::vector<FormDataPart> parts;
// 	std::string sep = "--" + boundary;
// 	std::string end = sep + "--";

// 	std::vector<std::string> blocks = utils_parsing::split_on_substring(body, sep); 
// 	for (size_t i = 0; i < blocks.size(); ++i)
// 	{
// 		std::string block = blocks[i];
// 		if (block.empty() || block == "--" || block == "--\r\n")
// 			continue;

// 		std::istringstream stream(block);
// 		std::string line;
// 		std::string contentDisp, contentType, content;

// 		while (std::getline(stream, line))
// 		{
// 			if (line == "\r" || line.empty()) 
// 				break;

// 			if (line.find("Content-Disposition:") == 0)
// 				contentDisp = line;
// 			else if (line.find("Content-Type:") == 0)
// 				contentType = line.substr(14); 
// 		}
// 		std::string contentLine;
// 		while (std::getline(stream, contentLine))
// 		{
// 			if (!content.empty())
// 				content += "\n";
// 			content += contentLine;
// 		}
// 		if (content.size() >= 2 && content.substr(content.size() - 2) == "\r\n")
// 			content = content.substr(0, content.size() - 2);

// 		std::map<std::string, std::string> dispMap = Request::parseContentDisposition(contentDisp);

// 		FormDataPart part;
// 		part.name = dispMap["name"];
// 		part.filename = dispMap["filename"];
// 		part.contentType = contentType;
// 		part.content = content;
// 		part.contentLength = content.size();

// 		parts.push_back(part);
// 	}

// 	return parts;
// }



std::vector<FormDataPart>
Request::parseMultipartFormData(const std::string& body, const std::string& boundary)
{
	std::vector<FormDataPart> parts;
	std::string sep = "--" + boundary;
	std::string end = sep + "--";
	std::cout << YELLOW << "[DEBUG multipart] boundary: " << boundary << RESET << std::endl;
	std::cout << CYAN << "[DEBUG multipart] body raw:\n" << body << "\n" << RESET << std::endl;

	size_t start = 0;
	while (true)
	{
		// chercher le début du prochain bloc
		size_t partStart = body.find(sep, start);
		if (partStart == std::string::npos)
			break;

		partStart += sep.size();

		// fin du flux ?
		if (body.compare(partStart, 2, "--") == 0)
			break;

		// chercher fin du bloc
		size_t nextPart = body.find(sep, partStart);
		if (nextPart == std::string::npos)
			nextPart = body.size();

		std::string block = body.substr(partStart, nextPart - partStart);

		// trim éventuel du \r\n en tête
		while (block.size() > 0 && (block[0] == '\r' || block[0] == '\n'))
			block.erase(0, 1);

		// trouver la séparation headers / contenu
		size_t headerEnd = block.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
			continue; // bloc invalide

		std::string headerPart = block.substr(0, headerEnd);
		std::string contentPart = block.substr(headerEnd + 4);

		// retirer le CRLF final éventuel avant le prochain boundary
		while (!contentPart.empty() && 
			   (contentPart[contentPart.size() - 1] == '\r' || contentPart[contentPart.size() - 1] == '\n'))
			contentPart.erase(contentPart.size() - 1);

		// --- extraire headers ---
		std::istringstream headerStream(headerPart);
		std::string line, contentDisp, contentType;
		while (std::getline(headerStream, line))
		{
			if (line.find("Content-Disposition:") == 0)
				contentDisp = line;
			else if (line.find("Content-Type:") == 0)
			{
				contentType = line.substr(14);
				// nettoyage du \r éventuel en fin de ligne
				if (!contentType.empty() && contentType[contentType.size() - 1] == '\r')
					contentType.erase(contentType.size() - 1);
			}
		}

		std::map<std::string, std::string> dispMap = Request::parseContentDisposition(contentDisp);

		FormDataPart part;
		part.name = dispMap["name"];
		part.filename = dispMap["filename"];
		part.contentType = contentType;
		part.content = contentPart;
		part.contentLength = contentPart.size();
		std::cout << GREEN << "[DEBUG multipart] Found part:" << RESET << std::endl;
		std::cout << "  name=" << part.name << "\n";
		std::cout << "  filename=" << part.filename << "\n";
		std::cout << "  type=" << part.contentType << "\n";
		std::cout << "  length=" << part.contentLength << "\n";

		parts.push_back(part);
		start = nextPart;
	}

	return parts;
}
