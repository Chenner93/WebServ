/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:57:19 by kahoumou          #+#    #+#             */
/*   Updated: 2025/10/28 18:20:58 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

 #include "../../includes/Request/Response.hpp"
 #include "../../includes/Server.hpp"


// std::string Response::Methodes(const Request &request) 
// {
//     if (request.getMethod() == "GET")
//                 return handleGet(request); 
//     else if (request.getMethod() == "POST")
//                 return handlePost(request);
//     else if (request.getMethod() == "DELETE")
//     {
//         std::cout<<CYAN<<"pass in METHODE delete"<<RESET<<std::endl;
//                return handleDelete(request);
//     }
//     else
//         return Response::sendError(405, "Method Not Allowed");
//     return("");
// }


    
//     std::string getContentType(const std::string& path)
//     {
//         size_t dotPos = path.rfind('.');
//         if (dotPos == std::string::npos)
//             return "application/octet-stream"; 
        
//         std::string ext = path.substr(dotPos + 1);
//         std::map<std::string, std::string> mimeTypes;
    
//         if(mimeTypes.empty())
//         {
//             mimeTypes["html"] = "text/html";
//             mimeTypes["htm"]  = "text/html";
//             mimeTypes["css"]  = "text/css";
//             mimeTypes["js"]   = "application/javascript";
//             mimeTypes["json"] = "application/json";
//             mimeTypes["png"]  = "image/png";
//             mimeTypes["jpg"]  = "image/jpeg";
//             mimeTypes["jpeg"] = "image/jpeg";
//             mimeTypes["gif"]  = "image/gif";
//             mimeTypes["svg"]  = "image/svg+xml";
//             mimeTypes["txt"]  = "text/plain";
//             mimeTypes["pdf"]  = "application/pdf";
//             mimeTypes["ico"]  = "image/x-icon";
//         }
    
//         if (mimeTypes.count(ext))
//             return mimeTypes[ext];
//         else
//             return "application/octet-stream";
//     }
//     /*
//     Status line : HTTP/1.1 200 OK
    
//     Header Content-Length : taille du body, obligatoire pour que le client sache quand s'arrête la réponse.
    
//     Header Content-Type: text/html : suppose que tu sers du HTML, mais cela peut être amélioré (voir ci-dessous).

// \r\n\r\n : séparation entre les headers et le corps.

// bodyStr : contenu du fichier.

// */

// std::string Response::handleGet(const Request &request) 
// {
//     std::string path = "www" + request.getPath();
//     std::cout<<GREEN<<"handleGet pass is "<<path<<RESET<<std::endl; 
//     std::string contentType = getContentType(path);
//     struct stat s;
//     if (stat(path.c_str(), &s) == -1)
//        return Response::sendError(404, "Not Found");


//     std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
//     if (!file)
//         return Response::sendError(500, "Internal Server Error");


//     std::ostringstream body;
//     body << file.rdbuf();
//     std::string bodyStr = body.str();


//     std::ostringstream response;
//     response << "HTTP/1.1 200 OK\r\n"
//     << "Content-Length: " << bodyStr.size() << "\r\n"
//     << "Content-Type: "<<contentType<<"\r\n"
//     << "\r\n"
//     << bodyStr;
//     return response.str();
// }



// void Response::saveFormDataToDisk(const Request::FormDataPart& part)
// {
//     std::cout<<GREEN<<"pass in  saveFormDataToDisk"<<RESET<<std::endl;
//     // Crée le chemin complet : uploads/hello.txt
//     std::string uploadPath = "uploads/" + part.filename;

//     std::ofstream file(uploadPath.c_str(), std::ios::binary);
//     if (!file)
//     {
//         std::cerr << RED << "Erreur ouverture fichier : " << uploadPath << RESET << std::endl;
//         return;
//     }

//     file << part.content;
//     file.close();

//     std::cout << GREEN << "[SAVED] " << uploadPath << RESET << std::endl;
// }



// std::string Response::sendErrorAsString(int code, const std::string& message)
// {
//     std::ostringstream response;
//     response << "HTTP/1.1 " << code << " " << message << "\r\n"
//              << "Content-Type: text/plain\r\n"
//              << "Content-Length: " << message.size() << "\r\n"
//              << "\r\n"
//              << message;
//     return response.str();
// }




// std::string Response::handlePost(const Request& request)
// {
//     std::cout << CYAN << "[DEBUG] → Entrée dans handlePost()" << RESET << std::endl;

//     std::string body = request.getBody();
//     const std::map<std::string, std::string>& headers = request.getHeaders();

//     std::cout << MAGENTA << "[DEBUG] Body in handlePost: [" << body << "]" << RESET << std::endl;

//     // Vérification multipart
//     if (headers.count("content-type") > 0 &&
//         headers.at("content-type").find("multipart/form-data") != std::string::npos)
//     {
//         std::cout << BLUE << "[DEBUG] → Type multipart/form-data détecté" << RESET << std::endl;

//         std::string boundary = Request::ParseBoundary(headers);
//         std::cout << YELLOW << "[DEBUG] Boundary extraite : [" << boundary << "]" << RESET << std::endl;

//         if (boundary.empty())
//         {
//             std::cout << RED << "[ERROR] → Boundary introuvable" << RESET << std::endl;
//             return sendError(400, "Boundary not found");
//         }

//         std::vector<Request::FormDataPart> parts = request.parseMultipartFormData(body, boundary);
//         std::cout << GREEN << "[DEBUG] Nombre de parties détectées : " << parts.size() << RESET << std::endl;

//         for (size_t i = 0; i < parts.size(); ++i)
//         {
//             std::cout << CYAN << "[DEBUG] → Partie " << i << " : name = " << parts[i].name << ", filename = " << parts[i].filename << RESET << std::endl;
//             std::cout << "[DEBUG] → Contenu : [" << parts[i].content << "]" << std::endl;

//             if (!parts[i].filename.empty())
//             {
//                 std::cout << GREEN << "[DEBUG] → Sauvegarde de la partie " << i << " dans un fichier" << RESET << std::endl;
//                 saveFormDataToDisk(parts[i]);
//             }
//             else
//             {
//                 std::cout << YELLOW << "[DEBUG] → Partie " << i << " ignorée (pas de filename)" << RESET << std::endl;
//             }
//         }

//         std::string msg = "Upload terminé avec succès.";
//         std::ostringstream response;
//         response << "HTTP/1.1 200 OK\r\n"
//                  << "Content-Type: text/plain\r\n"
//                  << "Content-Length: " << msg.size() << "\r\n"
//                  << "\r\n"
//                  << msg;

//         std::cout << CYAN << "[DEBUG] → Fin traitement multipart, réponse prête" << RESET << std::endl;
//         return response.str();
//     }

//     // Sinon, POST classique
//     std::cout << BLUE << "[DEBUG] → Requête POST classique (non multipart)" << RESET << std::endl;

//     std::ostringstream response;
//     response << "HTTP/1.1 200 OK\r\n"
//              << "Content-Type: text/plain\r\n"
//              << "Content-Length: " << body.size() << "\r\n"
//              << "\r\n"
//              << body;

//     std::cout << CYAN << "[DEBUG] → Réponse classique générée" << RESET << std::endl;
//     return response.str();
// }

// std::string Response::Methodes(const Request &request) 
// {
//     if (request.getMethod() == "GET")
//                 return handleGet(request); 
//     else if (request.getMethod() == "POST")
//                 return handlePost(request);
//     else if (request.getMethod() == "DELETE")
//     {
//         std::cout<<CYAN<<"pass in METHODE delete"<<RESET<<std::endl;
//                return handleDelete(request);
//     }
//     else
//         return Response::sendError(405, "Method Not Allowed");
//     return("");
// }


    
//     std::string getContentType(const std::string& path)
//     {
//         size_t dotPos = path.rfind('.');
//         if (dotPos == std::string::npos)
//             return "application/octet-stream"; 
        
//         std::string ext = path.substr(dotPos + 1);
//         std::map<std::string, std::string> mimeTypes;
    
//         if(mimeTypes.empty())
//         {
//             mimeTypes["html"] = "text/html";
//             mimeTypes["htm"]  = "text/html";
//             mimeTypes["css"]  = "text/css";
//             mimeTypes["js"]   = "application/javascript";
//             mimeTypes["json"] = "application/json";
//             mimeTypes["png"]  = "image/png";
//             mimeTypes["jpg"]  = "image/jpeg";
//             mimeTypes["jpeg"] = "image/jpeg";
//             mimeTypes["gif"]  = "image/gif";
//             mimeTypes["svg"]  = "image/svg+xml";
//             mimeTypes["txt"]  = "text/plain";
//             mimeTypes["pdf"]  = "application/pdf";
//             mimeTypes["ico"]  = "image/x-icon";
//         }
    
//         if (mimeTypes.count(ext))
//             return mimeTypes[ext];
//         else
//             return "application/octet-stream";
//     }
//     /*
//     Status line : HTTP/1.1 200 OK
    
//     Header Content-Length : taille du body, obligatoire pour que le client sache quand s'arrête la réponse.
    
//     Header Content-Type: text/html : suppose que tu sers du HTML, mais cela peut être amélioré (voir ci-dessous).

// \r\n\r\n : séparation entre les headers et le corps.

// bodyStr : contenu du fichier.

// */

// std::string Response::handleGet(const Request &request) 
// {
//     std::string path = "www" + request.getPath();
//     std::cout<<GREEN<<"handleGet pass is "<<path<<RESET<<std::endl; 
//     std::string contentType = getContentType(path);
//     struct stat s;
//     if (stat(path.c_str(), &s) == -1)
//        return Response::sendError(404, "Not Found");


//     std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
//     if (!file)
//         return Response::sendError(500, "Internal Server Error");


//     std::ostringstream body;
//     body << file.rdbuf();
//     std::string bodyStr = body.str();


//     std::ostringstream response;
//     response << "HTTP/1.1 200 OK\r\n"
//     << "Content-Length: " << bodyStr.size() << "\r\n"
//     << "Content-Type: "<<contentType<<"\r\n"
//     << "\r\n"
//     << bodyStr;
//     return response.str();
// }



// void Response::saveFormDataToDisk(const Request::FormDataPart& part)
// {
//     std::cout<<GREEN<<"pass in  saveFormDataToDisk"<<RESET<<std::endl;
//     // Crée le chemin complet : uploads/hello.txt
//     std::string uploadPath = "uploads/" + part.filename;

//     std::ofstream file(uploadPath.c_str(), std::ios::binary);
//     if (!file)
//     {
//         std::cerr << RED << "Erreur ouverture fichier : " << uploadPath << RESET << std::endl;
//         return;
//     }

//     file << part.content;
//     file.close();

//     std::cout << GREEN << "[SAVED] " << uploadPath << RESET << std::endl;
// }



// std::string Response::sendErrorAsString(int code, const std::string& message)
// {
//     std::ostringstream response;
//     response << "HTTP/1.1 " << code << " " << message << "\r\n"
//              << "Content-Type: text/plain\r\n"
//              << "Content-Length: " << message.size() << "\r\n"
//              << "\r\n"
//              << message;
//     return response.str();
// }




// std::string Response::handlePost(const Request& request)
// {
//     std::cout << CYAN << "[DEBUG] → Entrée dans handlePost()" << RESET << std::endl;

//     std::string body = request.getBody();
//     const std::map<std::string, std::string>& headers = request.getHeaders();

//     std::cout << MAGENTA << "[DEBUG] Body in handlePost: [" << body << "]" << RESET << std::endl;

//     // Vérification multipart
//     if (headers.count("content-type") > 0 &&
//         headers.at("content-type").find("multipart/form-data") != std::string::npos)
//     {
//         std::cout << BLUE << "[DEBUG] → Type multipart/form-data détecté" << RESET << std::endl;

//         std::string boundary = Request::ParseBoundary(headers);
//         std::cout << YELLOW << "[DEBUG] Boundary extraite : [" << boundary << "]" << RESET << std::endl;

//         if (boundary.empty())
//         {
//             std::cout << RED << "[ERROR] → Boundary introuvable" << RESET << std::endl;
//             return sendError(400, "Boundary not found");
//         }

//         std::vector<Request::FormDataPart> parts = request.parseMultipartFormData(body, boundary);
//         std::cout << GREEN << "[DEBUG] Nombre de parties détectées : " << parts.size() << RESET << std::endl;

//         for (size_t i = 0; i < parts.size(); ++i)
//         {
//             std::cout << CYAN << "[DEBUG] → Partie " << i << " : name = " << parts[i].name << ", filename = " << parts[i].filename << RESET << std::endl;
//             std::cout << "[DEBUG] → Contenu : [" << parts[i].content << "]" << std::endl;

//             if (!parts[i].filename.empty())
//             {
//                 std::cout << GREEN << "[DEBUG] → Sauvegarde de la partie " << i << " dans un fichier" << RESET << std::endl;
//                 saveFormDataToDisk(parts[i]);
//             }
//             else
//             {
//                 std::cout << YELLOW << "[DEBUG] → Partie " << i << " ignorée (pas de filename)" << RESET << std::endl;
//             }
//         }

//         std::string msg = "Upload terminé avec succès.";
//         std::ostringstream response;
//         response << "HTTP/1.1 200 OK\r\n"
//                  << "Content-Type: text/plain\r\n"
//                  << "Content-Length: " << msg.size() << "\r\n"
//                  << "\r\n"
//                  << msg;

//         std::cout << CYAN << "[DEBUG] → Fin traitement multipart, réponse prête" << RESET << std::endl;
//         return response.str();
//     }

//     // Sinon, POST classique
//     std::cout << BLUE << "[DEBUG] → Requête POST classique (non multipart)" << RESET << std::endl;

//     std::ostringstream response;
//     response << "HTTP/1.1 200 OK\r\n"
//              << "Content-Type: text/plain\r\n"
//              << "Content-Length: " << body.size() << "\r\n"
//              << "\r\n"
//              << body;

//     std::cout << CYAN << "[DEBUG] → Réponse classique générée" << RESET << std::endl;
//     return response.str();
// }


// std::string  Response::handleDelete(const  Request& request)
// {
//     std::string path  = "www" +  request.getPath();
//     struct stat s;
//     if(stat(path.c_str(), &s) == -1)
//     {
//          return sendError(404, "File not found");
//     }
    
//     if(remove(path.c_str()) == 0)
//     {
//         std::cout<<YELLOW<<"pass  in remove if beggin"<<RESET<<std::endl;
//             std::string msg = "file  deleted sucessfully\n";
    
   
//         std::ostringstream response;
//         response << "HTTP/1.1 200 OK\r\n"
//                  << "Content-Length: " << msg.size() << "\r\n"
//                  << "Content-Type: text/plain\r\n" << "\r\n"
//                  << msg;
//         return(response.str());   
//     }
//     else
//     {
//        return(sendError(500 , "could not delete file"));
//     }
//     return("");
// }

// std::string Response::sendError(int code, const std::string& msg)
// {
//     std::ostringstream response;
//     std::cerr << "[Error] HTTP " << code << ": " << msg << std::endl;

//     response << "HTTP/1.1 " << code << " " << msg << "\r\n"
//              << "Content-Type: text/plain\r\n"
//              << "Content-Length: " << msg.size() << "\r\n"
//              << "\r\n"
//              << msg;
//     return response.str();
// }

// // parse_response

// // create get_next_line  check size 
// std::string  Response::handleDelete(const  Request& request)
// {
//     std::string path  = "www" +  request.getPath();
//     struct stat s;
//     if(stat(path.c_str(), &s) == -1)
//     {
//          return sendError(404, "File not found");
//     }
    
//     if(remove(path.c_str()) == 0)
//     {
//         std::cout<<YELLOW<<"pass  in remove if beggin"<<RESET<<std::endl;
//             std::string msg = "file  deleted sucessfully\n";
    
   
//         std::ostringstream response;
//         response << "HTTP/1.1 200 OK\r\n"
//                  << "Content-Length: " << msg.size() << "\r\n"
//                  << "Content-Type: text/plain\r\n" << "\r\n"
//                  << msg;
//         return(response.str());   
//     }
//     else
//     {
//        return(sendError(500 , "could not delete file"));
//     }
//     return("");
// }

// std::string Response::sendError(int code, const std::string& msg)
// {
//     std::ostringstream response;
//     std::cerr << "[Error] HTTP " << code << ": " << msg << std::endl;

//     response << "HTTP/1.1 " << code << " " << msg << "\r\n"
//              << "Content-Type: text/plain\r\n"
//              << "Content-Length: " << msg.size() << "\r\n"
//              << "\r\n"
//              << msg;
//     return response.str();
// }

// // parse_response

// // create get_next_line  check size 


// Retire le préfixe location du path HTTP pour obtenir le chemin relatif
static std::string strip_location_prefix(const std::string& path, const std::string& loc_prefix)
{
    if (loc_prefix.empty()) return path;
    if (path.compare(0, loc_prefix.size(), loc_prefix) == 0)
    {
        std::string rel = path.substr(loc_prefix.size());
        if (rel.empty() || rel[0] != '/') rel = "/" + rel;
        return rel;
    }
    return path;
}

static std::string join_path(const std::string& a, const std::string& b)
{
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a[a.size()-1] == '/' && b[0] == '/') return a + b.substr(1);
    if (a[a.size()-1] != '/' && b[0] != '/') return a + "/" + b;
    return a + b;
}




// response.cpp
std::string Response::Methodes(const Request &request, const Server &server)
{
    // 405 si méthode non autorisée par la location
    if (!server.isMethodAllowed(request.getPath(), request.getMethod()))
        return sendError(405, "Method Not Allowed");

    if (request.getMethod() == "GET")
        return handleGet(request, server);
    else if (request.getMethod() == "POST")
        return handlePost(request, server);
    else if (request.getMethod() == "DELETE")
        return handleDelete(request, server);

    return sendError(501, "Not Implemented");
}


std::string Response::handleGet(const Request &request, const Server &server)
{
    const std::string& path = request.getPath();

    int loc = server.findLocationIndex(path);
    if (loc == -1) return sendError(404, "Not Found");

    // Redirection (si définie)
    const std::string& redir = server.getRedirect(loc);
    if (!redir.empty()) {
        std::ostringstream r;
        r << "HTTP/1.1 301 Moved Permanently\r\n"
          << "Location: " << redir << "\r\n"
          << "Content-Length: 0\r\n\r\n";
        return r.str();
    }

    // Résolution FS: root + (path sans préfixe de la location)
    std::string root   = server.getRoot(loc);
    std::string rel    = strip_location_prefix(path, server.getLocationPath(loc));
    std::string fsPath = join_path(root, rel);

    struct stat st;
    if (stat(fsPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        const std::string& index = server.getIndexFile(loc);
        if (!index.empty()) {
            std::string cand = join_path(fsPath, index);
            if (stat(cand.c_str(), &st) == 0 && S_ISREG(st.st_mode))
                fsPath = cand;
            else if (server.getAutoindex(loc))
            {
                std::cout<<RED<<"403 = 1"<<std::endl;
                return sendError(403, "Autoindex not implemented"); // à faire si tu veux
            }
            else
            {
                std::cout<<RED<<"403 = 1"<<std::endl;
                return sendError(403, "Forbidden");
            }
        } else if (server.getAutoindex(loc)) {
            std::cout<<RED<<"403 = 1"<<std::endl;
            return sendError(403, "Autoindex not implemented");
        } else {
            std::cout<<RED<<"403 = 1"<<std::endl;
            return sendError(403, "Forbidden");
        }
    }

    std::ifstream file(fsPath.c_str(), std::ios::binary);
    if (!file.is_open()) return sendError(404, "Not Found");

    std::ostringstream body; body << file.rdbuf();
    std::string bodyStr = body.str();

    std::string ctype = getContentType(fsPath);
    std::ostringstream res;
    res << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: "   << ctype          << "\r\n"
        << "Content-Length: " << bodyStr.size() << "\r\n"
        << "Connection: keep-alive\r\n"
        << "\r\n"
        << bodyStr;
    return res.str();
}



std::string Response::handlePost(const Request &request, const Server &server)
{
    const std::string& path = request.getPath();
    int loc = server.findLocationIndex(path);
    if (loc == -1) return sendError(404, "Not Found");

    if (!server.getUploadEnabled(loc))
        return sendError(403, "Uploads not allowed");

    const std::string& uploadDir = server.getUploadPath(loc);
    if (uploadDir.empty())
        return sendError(500, "Upload path not configured");

    // 413 si le body (non-chunked) dépasse client_max_body_size
    const std::map<std::string, std::string>& headers = request.getHeaders();
    std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
    if (it != headers.end()) {
        size_t cl = std::strtoul(it->second.c_str(), 0, 10);
        if (cl > server.getClientMaxBodySize())
            return sendError(413, "Payload Too Large");
    }

    // Multipart
    if (headers.count("content-type") &&
        headers.at("content-type").find("multipart/form-data") != std::string::npos)
    {
        std::string boundary = Request::ParseBoundary(headers);
        if (boundary.empty())
            return sendError(400, "Bad Request");

        std::vector<Request::FormDataPart> parts =
            Request::parseMultipartFormData(request.getBody(), boundary);

        for (size_t i = 0; i < parts.size(); ++i) {
            if (!parts[i].filename.empty()) {
                // Sauvegarde dans le répertoire configuré
                saveFormDataToDisk(parts[i], uploadDir);
            }
        }

        const std::string msg = "Upload terminé avec succès.";
        std::ostringstream r;
        r << "HTTP/1.1 200 OK\r\n"
          << "Content-Type: text/plain\r\n"
          << "Content-Length: " << msg.size() << "\r\n"
          << "\r\n"
          << msg;
        return r.str();
    }

    // POST non-multipart : on écrit le body brut dans un fichier daté/unique
    std::string filename = "upload.bin";
    {
        // facultatif: générer un nom unique
        static size_t n = 0;
        std::ostringstream oss; oss << "upload_" << ++n << ".bin";
        filename = oss.str();
    }

    std::string full = join_path(uploadDir, filename);
    std::ofstream out(full.c_str(), std::ios::binary);
    if (!out.is_open())
        return sendError(500, "Internal Server Error");
    out << request.getBody();
    out.close();

    std::ostringstream r;
    r << "HTTP/1.1 201 Created\r\n"
      << "Content-Length: 0\r\n"
      << "Connection: keep-alive\r\n\r\n";
    return r.str();
}

std::string Response::handleDelete(const Request &request, const Server &server)
{
    const std::string& path = request.getPath();
    int loc = server.findLocationIndex(path);
    if (loc == -1) return sendError(404, "Not Found");

    std::string root   = server.getRoot(loc);
    std::string rel    = strip_location_prefix(path, server.getLocationPath(loc));
    std::string target = join_path(root, rel);

    struct stat st;
    if (stat(target.c_str(), &st) == -1 || !S_ISREG(st.st_mode))
        return sendError(404, "File Not Found");

    if (remove(target.c_str()) != 0)
        return sendError(500, "Internal Server Error");

    std::ostringstream r;
    r << "HTTP/1.1 200 OK\r\n"
      << "Content-Length: 0\r\n"
      << "Connection: keep-alive\r\n\r\n";
    return r.str();
}

void Response::saveFormDataToDisk(const Request::FormDataPart& part,
                                  const std::string& upload_dir)
{
    std::string safe = part.filename;
    if (safe.find('/') != std::string::npos)   safe = safe.substr(safe.find_last_of('/') + 1);
    if (safe.find('\\') != std::string::npos)  safe = safe.substr(safe.find_last_of('\\') + 1);

    std::string full = join_path(upload_dir, safe);
    std::ofstream file(full.c_str(), std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("cannot open upload target");
    file << part.content;
    file.close();
}

std::string Response::sendError(int code, const std::string& msg)
{
	std::ostringstream response;
	std::cerr << RED << "[HTTP " << code << "] " << msg << RESET << std::endl;

	response << "HTTP/1.1 " << code << " " << msg << "\r\n"
			 << "Content-Type: text/plain\r\n"
			 << "Content-Length: " << msg.size() << "\r\n\r\n"
			 << msg;
	return response.str();
}


std::string Response::getContentType(const std::string& path)
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
	return "application/octet-stream";
}