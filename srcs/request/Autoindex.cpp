#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include "../../includes/Request/Response.hpp"
#include <Server.hpp>

// Structure pour stocker les informations d'un fichier/dossier
struct FileInfo {
	std::string name;
	bool isDirectory;
	off_t size;
	time_t modTime;
};

// Fonction de comparaison pour trier (dossiers en premier, puis alphabétique)
bool compareFileInfo(const FileInfo& a, const FileInfo& b) {
	if (a.isDirectory != b.isDirectory)
		return a.isDirectory;
	return a.name < b.name;
}

// Fonction pour convertir un nombre en string
std::string toString(long long value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

// Fonction pour formater la taille des fichiers
std::string formatFileSize(off_t bytes) {
	if (bytes < 1024)
		return toString(bytes) + " B";
	else if (bytes < 1024 * 1024) {
		double kb = bytes / 1024.0;
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(1) << kb << " KB";
		return oss.str();
	} else if (bytes < 1024 * 1024 * 1024) {
		double mb = bytes / (1024.0 * 1024.0);
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(1) << mb << " MB";
		return oss.str();
	} else {
		double gb = bytes / (1024.0 * 1024.0 * 1024.0);
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(1) << gb << " GB";
		return oss.str();
	}
}

// Fonction pour formater la date
std::string formatDateTime(time_t timestamp) {
	char buffer[80];
	struct tm* timeinfo = localtime(&timestamp);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
	return std::string(buffer);
}

// Fonction principale pour générer l'autoindex
std::string Response::generateAutoindex(const std::string& dirPath, const std::string& requestPath, const Server  &server) {
	DIR* dir = opendir(dirPath.c_str());
	if (!dir) {
		return sendError(500, "Internal Server Error", server);
	}

	std::vector<FileInfo> files;
	struct dirent* entry;

	// Lire tous les fichiers/dossiers
	while ((entry = readdir(dir)) != NULL) {
		std::string filename = entry->d_name;
		
		// Ignorer . et ..
		if (filename == "." || filename == "..")
			continue;

		FileInfo fileInfo;
		fileInfo.name = filename;

		// Obtenir les infos du fichier
		std::string fullPath = dirPath;
		if (fullPath[fullPath.length() - 1] != '/')
			fullPath += "/";
		fullPath += filename;

		struct stat st;
		if (stat(fullPath.c_str(), &st) == 0) {
			fileInfo.isDirectory = S_ISDIR(st.st_mode);
			fileInfo.size = st.st_size;
			fileInfo.modTime = st.st_mtime;
			files.push_back(fileInfo);
		}
	}
	closedir(dir);

	// Trier les fichiers
	std::sort(files.begin(), files.end(), compareFileInfo);

	// Générer le HTML
	std::ostringstream html;
	html << "<!DOCTYPE html>\n"
		 << "<html lang=\"fr\">\n"
		 << "<head>\n"
		 << "    <meta charset=\"UTF-8\">\n"
		 << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		 << "    <title>Index of " << requestPath << "</title>\n"
		 << "    <style>\n"
		 << "        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
		 << "        body {\n"
		 << "            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;\n"
		 << "            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
		 << "            min-height: 100vh;\n"
		 << "            padding: 2rem;\n"
		 << "        }\n"
		 << "        .container {\n"
		 << "            max-width: 1200px;\n"
		 << "            margin: 0 auto;\n"
		 << "            background: white;\n"
		 << "            border-radius: 20px;\n"
		 << "            box-shadow: 0 20px 60px rgba(0,0,0,0.3);\n"
		 << "            overflow: hidden;\n"
		 << "        }\n"
		 << "        header {\n"
		 << "            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
		 << "            color: white;\n"
		 << "            padding: 2rem;\n"
		 << "        }\n"
		 << "        h1 {\n"
		 << "            font-size: 2rem;\n"
		 << "            margin-bottom: 0.5rem;\n"
		 << "        }\n"
		 << "        .path {\n"
		 << "            opacity: 0.9;\n"
		 << "            font-size: 1.1rem;\n"
		 << "        }\n"
		 << "        table {\n"
		 << "            width: 100%;\n"
		 << "            border-collapse: collapse;\n"
		 << "        }\n"
		 << "        thead {\n"
		 << "            background: #f8f9fa;\n"
		 << "            border-bottom: 2px solid #dee2e6;\n"
		 << "        }\n"
		 << "        th {\n"
		 << "            text-align: left;\n"
		 << "            padding: 1rem;\n"
		 << "            font-weight: 600;\n"
		 << "            color: #495057;\n"
		 << "        }\n"
		 << "        td {\n"
		 << "            padding: 1rem;\n"
		 << "            border-bottom: 1px solid #dee2e6;\n"
		 << "        }\n"
		 << "        tr:hover {\n"
		 << "            background: #f8f9fa;\n"
		 << "        }\n"
		 << "        a {\n"
		 << "            color: #667eea;\n"
		 << "            text-decoration: none;\n"
		 << "            display: flex;\n"
		 << "            align-items: center;\n"
		 << "            gap: 0.5rem;\n"
		 << "        }\n"
		 << "        a:hover {\n"
		 << "            color: #764ba2;\n"
		 << "            text-decoration: underline;\n"
		 << "        }\n"
		 << "        .icon {\n"
		 << "            font-size: 1.5rem;\n"
		 << "        }\n"
		 << "        .size, .date {\n"
		 << "            color: #6c757d;\n"
		 << "        }\n"
		 << "        .back-link {\n"
		 << "            display: inline-block;\n"
		 << "            margin: 1.5rem 2rem;\n"
		 << "            padding: 0.75rem 1.5rem;\n"
		 << "            background: #667eea;\n"
		 << "            color: white;\n"
		 << "            border-radius: 25px;\n"
		 << "            transition: all 0.3s ease;\n"
		 << "        }\n"
		 << "        .back-link:hover {\n"
		 << "            background: #764ba2;\n"
		 << "            transform: translateY(-2px);\n"
		 << "            box-shadow: 0 5px 15px rgba(102,126,234,0.4);\n"
		 << "        }\n"
		 << "        footer {\n"
		 << "            padding: 1.5rem 2rem;\n"
		 << "            text-align: center;\n"
		 << "            color: #6c757d;\n"
		 << "            border-top: 1px solid #dee2e6;\n"
		 << "        }\n"
		 << "    </style>\n"
		 << "</head>\n"
		 << "<body>\n"
		 << "    <div class=\"container\">\n"
		 << "        <header>\n"
		 << "            <h1>📂 Index of Directory</h1>\n"
		 << "            <div class=\"path\">" << requestPath << "</div>\n"
		 << "        </header>\n";

	// Lien parent si pas à la racine
	if (requestPath != "/") {
		std::string parentPath = requestPath;
		size_t lastSlash = parentPath.rfind('/');
		if (lastSlash != std::string::npos && lastSlash > 0)
			parentPath = parentPath.substr(0, lastSlash);
		else
			parentPath = "/";
		
		html << "        <a href=\"" << parentPath << "\" class=\"back-link\">← Parent Directory</a>\n";
	}

	html << "        <table>\n"
		 << "            <thead>\n"
		 << "                <tr>\n"
		 << "                    <th>Name</th>\n"
		 << "                    <th>Size</th>\n"
		 << "                    <th>Last Modified</th>\n"
		 << "                </tr>\n"
		 << "            </thead>\n"
		 << "            <tbody>\n";

	// Ajouter chaque fichier/dossier
	for (size_t i = 0; i < files.size(); ++i) {
		const FileInfo& file = files[i];
		
		std::string url = requestPath;
		if (url[url.length() - 1] != '/')
			url += "/";
		url += file.name;

		std::string icon = file.isDirectory ? "📁" : "📄";
		std::string sizeStr = file.isDirectory ? "-" : formatFileSize(file.size);
		std::string dateStr = formatDateTime(file.modTime);

		html << "                <tr>\n"
			 << "                    <td><a href=\"" << url << "\"><span class=\"icon\">" 
			 << icon << "</span>" << file.name 
			 << (file.isDirectory ? "/" : "") << "</a></td>\n"
			 << "                    <td class=\"size\">" << sizeStr << "</td>\n"
			 << "                    <td class=\"date\">" << dateStr << "</td>\n"
			 << "                </tr>\n";
	}

	html << "            </tbody>\n"
		 << "        </table>\n"
		 << "        <footer>\n"
		 << "            <p>Webserv - Total: " << files.size() << " items</p>\n"
		 << "        </footer>\n"
		 << "    </div>\n"
		 << "</body>\n"
		 << "</html>";

	std::string body = html.str();
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/html; charset=utf-8\r\n"
			 << "Content-Length: " << body.size() << "\r\n"
			 << "Connection: keep-alive\r\n"
			 << "\r\n"
			 << body;

	return response.str();
}
