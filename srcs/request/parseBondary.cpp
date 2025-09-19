/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseBondary.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 13:54:19 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/19 14:22:10 by kahoumou         ###   ########.fr       */
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


// int main()
// {
//     std::map<std::string, std::string> headers;
//     headers["content-type"] = "multipart/form-data; boundary=----WebKitFormBoundaryABC123";

//     std::string boundary = Request::ParseBoundary(headers);

//     if (!boundary.empty())
//         std::cout << "Boundary extraite : [" << boundary << "]" << std::endl;
//     else
//         std::cout << "Aucune boundary trouvée." << std::endl;

//     return 0;
// }