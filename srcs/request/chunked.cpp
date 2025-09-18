/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 13:47:03 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/18 19:16:37 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Request/Request.hpp"





std::string Request::parseChunkedBody(const std::string& rawBody)
{
	std::string finalBody;
	std::string reparse_body;
	reparse_body = utils_parsing::remove_all_r(rawBody);
	// std::cout<<GREEN<<"reparse body"<<reparse_body<<RESET<<std::endl;
	std::vector<std::string> lines = utils_parsing::ft_split(reparse_body, '\n');

	// std::cout << "[DEBUG] → Transfer-Encoding is chunked" << std::endl;

	// std::cout << "--- [DEBUG] Étape 1 : lignes brutes (avec \\n) ---" << std::endl;
	// for (size_t i = 0; i < lines.size(); ++i)
		// std::cout << "Line[" << i << "] = [" << lines[i] << "]" << std::endl;

	utils_parsing::clean_chunk_lines(lines); // Supprime \r, \t, espaces

	std::cout << "--- [DEBUG] Étape 2 : lignes nettoyées ---" << std::endl;
	for (size_t i = 0; i < lines.size(); ++i)
		std::cout << "Cleaned[" << i << "] = [" << lines[i] << "]" << std::endl;

	// Skip header parasite s'il y a un seul mot hexadécimal tout seul au début
	std::stringstream precheck;
	precheck << std::hex << lines[0];
	size_t tmpSize = 0;
	precheck >> tmpSize;
	if (!precheck.fail() && tmpSize > 20) // seuil arbitraire
	{
		// std::cout << "[WARN] Ligne d'entête suspecte ignorée : " << lines[0] << std::endl;
		lines.erase(lines.begin());
	}


	for (size_t i = 0; i < lines.size();)
	{
		std::string sizeLine = lines[i];
		std::stringstream ss;
		ss << std::hex << sizeLine;

		size_t chunkSize = 0;
		ss >> chunkSize;

		// std::cout << "--- [DEBUG] Étape 3 : Lecture taille chunk à l’index " << i << " ---" << std::endl;
		// std::cout << "Chunk size line: [" << sizeLine << "]" << std::endl;

		if (ss.fail())
		{
			std::cout << "⚠️ Ignoré (pas un nombre hexadécimal valide)" << std::endl;
			i++;
			continue;
		}

		if (chunkSize == 0)
		{
			// std::cout << "→ Chunk size == 0 : fin de transfert." << std::endl;
			break;
		}

		i++;
		if (i >= lines.size())
			break;

		std::string dataLine = lines[i];
		// std::cout << "Donnée brute reçue: [" << dataLine << "]" << std::endl;

		if (dataLine.length() > chunkSize)
			dataLine = dataLine.substr(0, chunkSize);

		// std::cout << "→ Chunk retenu: [" << dataLine << "]" << std::endl;
		finalBody += dataLine;
		i++;
	}

	// std::cout << "--- [DEBUG] Résultat final ---" << std::endl;
	// std::cout << "Chunked Body = [" << finalBody << "]" << std::endl;
	return finalBody;
}
