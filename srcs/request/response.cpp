/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:57:19 by kahoumou          #+#    #+#             */
/*   Updated: 2025/11/10 17:14:17 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Request/Response.hpp"
#include "../../includes/Server.hpp"

std::string strip_location_prefix(const std::string& path, const std::string& loc_prefix)
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

std::string join_path(const std::string& a, const std::string& b)
{
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a[a.size()-1] == '/' && b[0] == '/') return a + b.substr(1);
    if (a[a.size()-1] != '/' && b[0] != '/') return a + "/" + b;
    return a + b;
}


static void debug_headers(const std::map<std::string, std::string>& headers)
{
	std::cout << CYAN << "[DEBUG] Request Headers:" << RESET << std::endl;

	if (headers.empty())
	{
		std::cout << "  (aucun header)" << std::endl;
		return;
	}

	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
}

// // response.cpp
std::string Response::Methodes(const Request &request, const Server &server)
{
    
    std::cout<<YELLOW<<"PASS HERE in  Mehtode METHODE"<<std::endl;
    debug_headers(request.getHeaders());
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




// // Modification dans handleGet pour intégrer l'autoindex
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

	// Résolution FS
	std::string root   = server.getRoot(loc);
	std::string rel    = strip_location_prefix(path, server.getLocationPath(loc));
	std::string fsPath = join_path(root, rel);

	struct stat st;
	
	// Si c'est un répertoire
	if (stat(fsPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		const std::string& index = server.getIndexFile(loc);
		
		// 1. Chercher le fichier index d'abord
		if (!index.empty()) {
			std::string indexPath = join_path(fsPath, index);
			struct stat indexSt;
			
			if (stat(indexPath.c_str(), &indexSt) == 0 && S_ISREG(indexSt.st_mode)) {
				// Index trouvé ! Le servir (que autoindex soit on ou off)
				fsPath = indexPath;
				// Continue vers la section "Servir le fichier"
			} else {
				// 2. Pas d'index.html trouvé
				if (server.getAutoindex(loc)) {
					// autoindex ON → générer la liste
					return generateAutoindex(fsPath, path);
				} else {
					// autoindex OFF → erreur 403
					return sendError(403, "Forbidden");
				}
			}
		} else {
			// 3. Pas d'index configuré du tout
			if (server.getAutoindex(loc)) {
				// autoindex ON → générer la liste
				return generateAutoindex(fsPath, path);
			} else {
				// autoindex OFF → erreur 403
				return sendError(403, "Forbidden");
			}
		}
	}

	// Servir le fichier (index.html ou fichier quelconque)
	std::ifstream file(fsPath.c_str(), std::ios::binary);
	if (!file.is_open()) 
		return sendError(404, "Not Found");

	std::ostringstream body;
	body << file.rdbuf();
	std::string bodyStr = body.str();

	std::string ctype = getContentType(fsPath);
	std::ostringstream res;
	res << "HTTP/1.1 200 OK\r\n"
		<< "Content-Type: " << ctype << "\r\n"
		<< "Content-Length: " << bodyStr.size() << "\r\n"
		<< "Connection: keep-alive\r\n"
		<< "\r\n"
		<< bodyStr;
	return res.str();
}



// std::string Response::handlePost(const Request &request, const Server &server)
// {
//     const std::string& path = request.getPath();
//     int loc = server.findLocationIndex(path);
//     if (loc == -1) return sendError(404, "Not Found");

//     if (!server.getUploadEnabled(loc))
//         return sendError(403, "Uploads not allowed");

//     const std::string& uploadDir = server.getUploadPath(loc);
//     if (uploadDir.empty())
//         return sendError(500, "Upload path not configured");

//     // 413 si le body (non-chunked) dépasse client_max_body_size
//     const std::map<std::string, std::string>& headers = request.getHeaders();
//     std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
//     if (it != headers.end()) {
//         size_t cl = std::strtoul(it->second.c_str(), 0, 10);
//         if (cl > server.getClientMaxBodySize())
//             return sendError(413, "Payload Too Large");
//     }

//     // Multipart
//     if (headers.count("content-type") &&
//         headers.at("content-type").find("multipart/form-data") != std::string::npos)
//     {
//         std::string boundary = Request::ParseBoundary(headers);
//         if (boundary.empty())
//             return sendErro contentType;r(400, "Bad Request");

//         std::vector<Request::FormDataPart> parts =
//             Request::parseMultipartFormData(request.getBody(), boundary);

//         for (size_t i = 0; i < parts.size(); ++i) {
//             if (!parts[i].filename.empty()) {
//                 // Sauvegarde dans le répertoire configuré
//                 saveFormDataToDisk(parts[i], uploadDir);
//             }
//         }

//         const std::string msg = "Upload terminé avec succès.";
//         std::ostringstream r;
//         r << "HTTP/1.1 200 OK\r\n"
//           << "Content-Type: text/plain\r\n"
//           << "Content-Length: " << msg.size() << "\r\n"
//           << "\r\n"
//           << msg;
//         return r.str();
//     }

//     // POST non-multipart : on écrit le body brut dans un fichier daté/unique
//     std::string filename = "upload.bin";
//     {
//         // facultatif: générer un nom unique
//         static size_t n = 0;
//         std::ostringstream oss; oss << "upload_" << ++n << ".bin";
//         filename = oss.str();
//     }

//     std::string full = join_path(uploadDir, filename);
//     std::ofstream out(full.c_str(), std::ios::binary);
//     if (!out.is_open())
//         return sendError(500, "Internal Server Error");
//     out << request.getBody();
//     out.close();

//     std::ostringstream r;
//     r << "HTTP/1.1 201 Created\r\n"
//       << "Content-Length: 0\r\n"
//       << "Connection: keep-alive\r\n\r\n";
//     return r.str();
// }

std::string Response::resolveUploadPath(const Server &server, int loc, const std::string &filename) const
{
	std::string root = server.getRoot(loc);
	std::string uploadDir = server.getUploadPath(loc);
	std::string resolved;

	
	if (uploadDir.empty())
		resolved = root;
	else if (uploadDir[0] == '/' || (uploadDir.size() > 1 && uploadDir[0] == '.' && uploadDir[1] == '/'))
		resolved = uploadDir; // chemin complet
	else
		resolved = join_path(root, uploadDir); // relatif au root


	std::string safe = filename;
	if (safe.empty()) {
		std::string defaultName;
		if (!server.getIndexFile(loc).empty())
			defaultName = server.getIndexFile(loc);
		else
			defaultName = "default_upload"; 
	}

	
	if (safe.find('/') != std::string::npos)
		safe = safe.substr(safe.find_last_of('/') + 1);
	if (safe.find('\\') != std::string::npos)
		safe = safe.substr(safe.find_last_of('\\') + 1);
	resolved = join_path(resolved, safe);
	return resolved;
}

static std::string clean_boundary(const std::string& content_type)
{
    // Exemple: "multipart/form-data; boundary=------------------------6772a3885638bc48"
    std::string ct = content_type;
    std::string ctl = utils_parsing::to_lower(ct);

    size_t p = ctl.find("boundary=");
    if (p == std::string::npos)
        return "";

    std::string b = ct.substr(p + 9);

    
    b = utils_parsing::trim(b);
    while (!b.empty() && 
          (b[b.size() - 1] == '\r' || b[b.size() - 1] == '\n' || 
           b[b.size() - 1] == ' '  || b[b.size() - 1] == '\t'))
    {
        b.erase(b.size() - 1);
    }

    if (b.size() >= 2 && b[0] == '"' && b[b.size() - 1] == '"')
        b = b.substr(1, b.size() - 2);

    while (b.size() >= 2 && b.compare(0, 2, "--") == 0)
        b.erase(0, 2);

    return b;
}


std::string Response::handlePost(const Request &request, const Server &server)
{
    const std::string& path = request.getPath();
    int loc = server.findLocationIndex(path);
    if (loc == -1)
        return sendError(404, "Not Found");

    const std::vector<std::string>& methods = server.getAllowMethods(loc);
    if (std::find(methods.begin(), methods.end(), "POST") == methods.end())
        return sendError(405, "Method Not Allowed");

    const std::map<std::string, std::string>& headers = request.getHeaders();
    std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
    if (it != headers.end())
    {
        size_t cl = std::strtoul(it->second.c_str(), 0, 10);
        if (cl > server.getClientMaxBodySize())
            return sendError(413, "Payload Too Large");
    }

    if (!server.getUploadEnabled(loc))
        return sendError(403, "Uploads not allowed");

    const std::string& root = server.getRoot(loc);
    const std::string& uploadDir = server.getUploadPath(loc);
    std::string fullDir = uploadDir; 
    if (uploadDir.empty())
            fullDir = root;


    // std::string fullDir = join_path(root, uploadDir);

    struct stat st;
    if (stat(fullDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
    {
        std::cerr << RED << "[DEBUG] Upload directory missing → creating: "
                  << fullDir << RESET << std::endl;
        system(("mkdir -p " + fullDir).c_str());
    }

    if (headers.count("content-type") &&
        headers.at("content-type").find("multipart/form-data") != std::string::npos)
    {
        std::string raw_ct = headers.at("content-type");
        std::string boundary = clean_boundary(raw_ct);

        if (boundary.empty())
            return sendError(400, "Bad Request: Missing boundary");

        std::cout << YELLOW << "[DEBUG] Boundary nettoyé: [" << boundary << "]" << RESET << std::endl;

        std::vector<Request::FormDataPart> parts =
            Request::parseMultipartFormData(request.getBody(), boundary);

        if (parts.empty())
            return sendError(400, "Bad Request: No multipart data found");

        size_t saved = 0;
        for (size_t i = 0; i < parts.size(); ++i)
        {
            try {
                std::cout << GREEN << "[DEBUG] Saving part " << i + 1 << " / " << parts.size()
                          << ": " << parts[i].filename << RESET << std::endl;
                saveFormDataToDisk(parts[i], fullDir);
                ++saved;
            }
            catch (const std::exception& e)
            {
                std::cerr << RED << "[ERROR] Upload failed: " << e.what() << RESET << std::endl;
                return sendError(500, e.what());
            }
        }

        std::ostringstream msg;
        msg << saved << " fichier(s) uploadé(s) avec succès.";
        std::ostringstream r;
        r << "HTTP/1.1 201 Created\r\n"
          << "Content-Type: text/plain\r\n"
          << "Content-Length: " << msg.str().size() << "\r\n"
          << "\r\n"
          << msg.str();
        return r.str();
    }

    if (headers.count("content-type") &&
        headers.at("content-type").find("application/x-www-form-urlencoded") != std::string::npos)
    {
        std::string dest = join_path(fullDir, "form_data.txt");
        std::ofstream file(dest.c_str(), std::ios::app);
        if (!file.is_open())
            return sendError(500, "Cannot open upload target");

        file << request.getBody() << "\n";
        file.close();

        const std::string msg = "Form data saved.";
        std::ostringstream r;
        r << "HTTP/1.1 200 OK\r\n"
          << "Content-Type: text/plain\r\n"
          << "Content-Length: " << msg.size() << "\r\n"
          << "\r\n"
          << msg;
        return r.str();
    }

    return sendError(400, "Bad Request: Unsupported POST format");
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


// void Response::saveFormDataToDisk(const Request::FormDataPart& part,
//                                   const std::string& upload_dir)
// {
//     std::string safe = part.filename;
//     if (safe.find('/') != std::string::npos)   safe = safe.substr(safe.find_last_of('/') + 1);
//     if (safe.find('\\') != std::string::npos)  safe = safe.substr(safe.find_last_of('\\') + 1);

//     std::string full = join_path(upload_dir, safe);
//     std::ofstream file(full.c_str(), std::ios::binary);
//     if (!file.is_open())
//         throw std::runtime_error("cannot open upload target");
//     file << part.content;
//     file.close();
// }


void Response::saveFormDataToDisk(const Request::FormDataPart& part,
                                  const std::string& base_dir)
{
    std::string safe = part.filename;
    if (safe.empty())
    {
        std::ostringstream oss;
        oss << "upload_" << std::time(0) << ".bin";
        safe = oss.str();
    }

    if (safe.find('/') != std::string::npos)
        safe = safe.substr(safe.find_last_of('/') + 1);
    if (safe.find('\\') != std::string::npos)
        safe = safe.substr(safe.find_last_of('\\') + 1);

    std::string full = join_path(base_dir, safe);
    std::ofstream file(full.c_str(), std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("cannot open upload target");

    file << part.content;
    file.close();

    std::cout << GREEN << "[DEBUG] Saved file → " << full
              << " (" << part.content.size() << " bytes)" << RESET << std::endl;
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