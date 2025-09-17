/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 13:47:03 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/17 15:18:22 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Request/Request.hpp"



std::string Request::parseChunkedBody(const std::string& rawBody)
 {
        std::istringstream stream(rawBody);
        std::string finalBody;
        std::string line;
        
        while(std::getline(stream, line))
        {
            if(line.empty() || line == "\r")
             continue;
            std::stringstream sizeStream(line);
            std::size_t chunksize;
            sizeStream >> std::hex >> chunksize;
            if(chunksize == 0)
                break;
            
            std::string chunk(chunksize, '\0');
            stream.read(&chunk[0], chunksize);
            finalBody += chunk;
            stream.get();
            stream.get();
            std::cout << CYAN << "Chunk received: " << chunk << RESET << std::endl;
        }
        return(finalBody);    
 }