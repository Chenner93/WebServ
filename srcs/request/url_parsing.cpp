/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   url_parsing.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 16:03:43 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/11 14:06:01 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Request/Request.hpp"



void Request::parse_url()
{   
     std::size_t pos = this->path.find('?');
    if(pos == std::string::npos)
            return;
    std::string  request_str = this->path.substr(pos + 1);
    this->path = path.substr(0, pos);
       
    std::vector<std::string> cut_request = utils_parsing:: split_on_substr(request_str, "&", false);
       for(size_t i = 0; i  < cut_request.size(); ++i)
       {
            std::size_t equal = cut_request[i].find('=');
            if(equal ==  std::string::npos)
                continue;
            std::string key = utils_parsing::decode_url( utils_parsing::trim(cut_request[i].substr(0, equal)));
            std::string value = utils_parsing::decode_url(utils_parsing::trim(cut_request[i].substr(equal + 1)));
            url_params[key] = value;
       }                
}

const std::map<std::string, std::string>& Request:: getUrlparams()  const
{ 
    return(url_params);
}

bool  Request::hasUrlParam(const std::string& key) const
{
    return(url_params.find(key) != url_params.end());
}

std::string Request::getUrlParam(const std::string& key) const
{
     std::map<std::string, std::string>::const_iterator it =  url_params.find(key);
     if(it !=  url_params.end())
        return it->second;
    return "";
}




