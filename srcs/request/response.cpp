/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:57:19 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/23 14:03:47 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

 #include "../../includes/Request/Response.hpp"


std::string Response::Methodes(const Request &request) 
{
    if (request.getMethod() == "GET")
                return handleGet(request); 
    else if (request.getMethod() == "POST")
                return handlePost(request);
    else if (request.getMethod() == "DELETE")
    {
        std::cout<<CYAN<<"pass in METHODE delete"<<RESET<<std::endl;
               return handleDelete(request);
    }
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
    
        if(mimeTypes.empty())
        {
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
        }
    
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
    std::string path = "www" + request.getPath();
    std::cout<<GREEN<<"handleGet pass is "<<path<<RESET<<std::endl; 
    std::string contentType = getContentType(path);
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
    << "Content-Type: "<<contentType<<"\r\n"
    << "\r\n"
    << bodyStr;
    return response.str();
}



void Response::saveFormDataToDisk(const Request::FormDataPart& part)
{
    std::cout<<GREEN<<"pass in  saveFormDataToDisk"<<RESET<<std::endl;
    // Crée le chemin complet : uploads/hello.txt
    std::string uploadPath = "uploads/" + part.filename;

    std::ofstream file(uploadPath.c_str(), std::ios::binary);
    if (!file)
    {
        std::cerr << RED << "Erreur ouverture fichier : " << uploadPath << RESET << std::endl;
        return;
    }

    file << part.content;
    file.close();

    std::cout << GREEN << "[SAVED] " << uploadPath << RESET << std::endl;
}



std::string Response::sendErrorAsString(int code, const std::string& message)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << code << " " << message << "\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << message.size() << "\r\n"
             << "\r\n"
             << message;
    return response.str();
}




std::string Response::handlePost(const Request& request)
{
    std::cout << CYAN << "[DEBUG] → Entrée dans handlePost()" << RESET << std::endl;

    std::string body = request.getBody();
    const std::map<std::string, std::string>& headers = request.getHeaders();

    std::cout << MAGENTA << "[DEBUG] Body in handlePost: [" << body << "]" << RESET << std::endl;

    // Vérification multipart
    if (headers.count("content-type") > 0 &&
        headers.at("content-type").find("multipart/form-data") != std::string::npos)
    {
        std::cout << BLUE << "[DEBUG] → Type multipart/form-data détecté" << RESET << std::endl;

        std::string boundary = Request::ParseBoundary(headers);
        std::cout << YELLOW << "[DEBUG] Boundary extraite : [" << boundary << "]" << RESET << std::endl;

        if (boundary.empty())
        {
            std::cout << RED << "[ERROR] → Boundary introuvable" << RESET << std::endl;
            return sendError(400, "Boundary not found");
        }

        std::vector<Request::FormDataPart> parts = request.parseMultipartFormData(body, boundary);
        std::cout << GREEN << "[DEBUG] Nombre de parties détectées : " << parts.size() << RESET << std::endl;

        for (size_t i = 0; i < parts.size(); ++i)
        {
            std::cout << CYAN << "[DEBUG] → Partie " << i << " : name = " << parts[i].name << ", filename = " << parts[i].filename << RESET << std::endl;
            std::cout << "[DEBUG] → Contenu : [" << parts[i].content << "]" << std::endl;

            if (!parts[i].filename.empty())
            {
                std::cout << GREEN << "[DEBUG] → Sauvegarde de la partie " << i << " dans un fichier" << RESET << std::endl;
                saveFormDataToDisk(parts[i]);
            }
            else
            {
                std::cout << YELLOW << "[DEBUG] → Partie " << i << " ignorée (pas de filename)" << RESET << std::endl;
            }
        }

        std::string msg = "Upload terminé avec succès.";
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/plain\r\n"
                 << "Content-Length: " << msg.size() << "\r\n"
                 << "\r\n"
                 << msg;

        std::cout << CYAN << "[DEBUG] → Fin traitement multipart, réponse prête" << RESET << std::endl;
        return response.str();
    }

    // Sinon, POST classique
    std::cout << BLUE << "[DEBUG] → Requête POST classique (non multipart)" << RESET << std::endl;

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "\r\n"
             << body;

    std::cout << CYAN << "[DEBUG] → Réponse classique générée" << RESET << std::endl;
    return response.str();
}


std::string  Response::handleDelete(const  Request& request)
{
    std::string path  = "www" +  request.getPath();
    struct stat s;
    if(stat(path.c_str(), &s) == -1)
    {
         return sendError(404, "File not found");
    }
    
    if(remove(path.c_str()) == 0)
    {
        std::cout<<YELLOW<<"pass  in remove if beggin"<<RESET<<std::endl;
            std::string msg = "file  deleted sucessfully\n";
    
   
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Length: " << msg.size() << "\r\n"
                 << "Content-Type: text/plain\r\n" << "\r\n"
                 << msg;
        return(response.str());   
    }
    else
    {
       return(sendError(500 , "could not delete file"));
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
