/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thbasse <thbasse@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:57:19 by kahoumou          #+#    #+#             */
/*   Updated: 2025/11/03 10:07:03 by thbasse          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

 #include "../../includes/Request/Response.hpp"
 #include "../../includes/Server.hpp"


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
    std::cout<<YELLOW<<"PASS HERE in  Mehtode METHODE"<<std::endl;
    // 405 si méthode non autorisée par la location
    if (!server.isMethodAllowed(request.getPath(), request.getMethod()))
        return sendError(405, "Method Not Allowed");

    if (request.getMethod() == "GET")
        return handleGet(request, server);
    else if (request.getMethod() == "POST")
        return handlePost(request, server);
    else if (request.getMethod() == "DELETE")
        return handleDelete(request, server);
    // else if (request.getMethod() == "HEAD")
    //     return handleHead(request,server);
    
    return sendError(501, "Not Implemented");
}


std::string Response::handleGet(const Request &request, const Server &server)
{
    const std::string& path = request.getPath();
    int loc = server.findLocationIndex(path);

    if (loc == -1)
    {
        return sendError(404, "Not Found");
    }

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
            if (stat(fsPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
                    return sendError(404, "Not Found");
            else if (server.getAutoindex(loc))
            {
                std::cout<<RED<<"403 = 1"<<std::endl;
                return sendError(403, "Autoindex not implemented");
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
    if (!file.is_open()) 
            return sendError(404, "Not Found");

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

std::string Response::handleHead(const Request &request, const Server &server)
{
    const std::string& path = request.getPath();
    int loc = server.findLocationIndex(path);
    if (loc == -1)
        return sendError(404, "Not Found");

    std::string root   = server.getRoot(loc);
    std::string rel    = strip_location_prefix(path, server.getLocationPath(loc));
    std::string fsPath = join_path(root, rel);
    std::cout << CYAN
          << "root: "   << root   << "\n"
          << "rel: "    << rel    << "\n"
          << "fsPath: " << fsPath << RESET << std::endl;

    struct stat st;
    if (stat(fsPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
        return sendError(404, "Not Found");

    std::string ctype = getContentType(fsPath);

    std::ostringstream res;
    res << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: "   << ctype      << "\r\n"
        << "Content-Length: " << st.st_size << "\r\n"
        << "Connection: keep-alive\r\n\r\n";

    return res.str();
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