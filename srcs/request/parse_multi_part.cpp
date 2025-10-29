/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_multi_part.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/20 12:08:53 by kahoumou          #+#    #+#             */
/*   Updated: 2025/10/29 15:34:48 by kahoumou         ###   ########.fr       */
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


std::vector<Request::FormDataPart> Request::parseMultipartFormData(const std::string& body, const std::string& boundary)
{
	std::vector<FormDataPart> parts;
	std::string sep = "--" + boundary;
	std::string end = sep + "--";

	std::vector<std::string> blocks = utils_parsing::split_on_substring(body, sep); 
	for (size_t i = 0; i < blocks.size(); ++i)
	{
		std::string block = blocks[i];
		if (block.empty() || block == "--" || block == "--\r\n")
			continue;

		std::istringstream stream(block);
		std::string line;
		std::string contentDisp, contentType, content;

		while (std::getline(stream, line))
		{
			if (line == "\r" || line.empty()) 
				break;

			if (line.find("Content-Disposition:") == 0)
				contentDisp = line;
			else if (line.find("Content-Type:") == 0)
				contentType = line.substr(14); 
		}
		std::string contentLine;
		while (std::getline(stream, contentLine))
		{
			if (!content.empty())
				content += "\n";
			content += contentLine;
		}
		if (content.size() >= 2 && content.substr(content.size() - 2) == "\r\n")
			content = content.substr(0, content.size() - 2);

		std::map<std::string, std::string> dispMap = Request::parseContentDisposition(contentDisp);

		FormDataPart part;
		part.name = dispMap["name"];
		part.filename = dispMap["filename"];
		part.contentType = contentType;
		part.content = content;
		part.contentLength = content.size();

		parts.push_back(part);
	}

	return parts;
}
