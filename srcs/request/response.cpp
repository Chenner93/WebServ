/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:57:19 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/15 17:52:37 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

 #include "../../includes/Request/Response.hpp"


std::string Response::Methodes(const Request &request) 
{
    if (request.getMethod() == "GET")
                return handleGet(request); // cette méthode retourne std::string
    else if (request.getMethod() == "POST")
                return handlePost(request);
    else if (request.getMethod() == "DELETE")
               return handleDelete(request);
    else
        return Response::sendError(405, "Method Not Allowed");
    return("");
}


    
    std::string getContentType(const std::string& path)
    {
        size_t dotPos = path.rfind('.');
        if (dotPos == std::string::npos)
            return "application/octet-stream"; 
        std::string ext = path.substr(dotPos + 1);
        std::map<std::string, std::string> mimeTypes;
    
       
        mimeTypes["html"] = "text/html";
        mimeTypes["htm"]  = "text/html";
        mimeTypes["css"]  = "text/css";
        mimeTypes["js"]   = "application/javascript";
        mimeTypes["json"] = "application/json";
        mimeTypes["png"]  = "image/png";
        mimeTypes["jpg"]  = "image/jpeg";
        mimeTypes["jpeg"] = "image/jpeg";
        mimeTypes["gif"]  = "image/gif";
        mimeTypes["svg"]  = "image/svg+xml";
        mimeTypes["txt"]  = "text/plain";
        mimeTypes["pdf"]  = "application/pdf";
        mimeTypes["ico"]  = "image/x-icon";
    
        if (mimeTypes.count(ext))
            return mimeTypes[ext];
        else
            return "application/octet-stream";
    }
    /*
    Status line : HTTP/1.1 200 OK
    
    Header Content-Length : taille du body, obligatoire pour que le client sache quand s'arrête la réponse.
    
    Header Content-Type: text/html : suppose que tu sers du HTML, mais cela peut être amélioré (voir ci-dessous).

\r\n\r\n : séparation entre les headers et le corps.

bodyStr : contenu du fichier.

*/

std::string Response::handleGet(const Request &request) 
{
    std::string path = "." + request.getPath();
    struct stat s;
    if (stat(path.c_str(), &s) == -1)
       return Response::sendError(404, "Not Found");


    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file)
        return Response::sendError(500, "Internal Server Error");


    std::ostringstream body;
    body << file.rdbuf();
    std::string bodyStr = body.str();


    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
    << "Content-Length: " << bodyStr.size() << "\r\n"
    << "Content-Type: text/html\r\n"
    << "\r\n"
    << bodyStr;
    return response.str();
}



std::string  Response::handlePost(const Request& request)
{
    std::string body = request.getBody();
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n" 
    << "Content-Type: text/plain\r\n" <<
     "Content-Length: " << body.size()
      << "\r\n" << "\r\n" 
      << body;
   return(response.str());
}

std::string  Response::handleDelete(const  Request& request)
{
    std::string path  = "." +  request.getPath();
    if(remove(path.c_str()) == 0)
    {
            std::string msg = "file  deleted sucessfully";
    
     std::ostringstream response;
        response<<"HTTP/1.1 200 ok\r\n"
        <<"constent-Length:" <<msg.size()<<"\r\n"
        <<"Content-Type: text/pain\r\n"
        <<"\r\n"
        << msg;
        return(response.str());   
    }
    else
    {
       return(sendError(404 , "Not Found"));
    }
    return("");
}

std::string Response::sendError(int code, const std::string& msg)
{
    std::ostringstream response;
    std::cerr << "[Error] HTTP " << code << ": " << msg << std::endl;

    response << "HTTP/1.1 " << code << " " << msg << "\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << msg.size() << "\r\n"
             << "\r\n"
             << msg;
    return response.str();
}
