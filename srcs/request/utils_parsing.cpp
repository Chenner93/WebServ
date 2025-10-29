/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_parsing.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 15:05:27 by kahoumou          #+#    #+#             */
/*   Updated: 2025/10/29 15:40:31 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include"../../includes/Request/Utils_parsing.hpp"



std::vector<std::string>
	utils_parsing::split_on_substr(const std::string& s, const std::string& delim, bool keep_empty)
{
    std::vector<std::string> out;
    if (delim.empty()) { out.push_back(s); return out; }

    std::size_t start = 0;
    while (true) {
        std::size_t pos = s.find(delim, start);
        if (pos == std::string::npos) {
            if (start < s.size() || keep_empty)
                out.push_back(s.substr(start));
            break;
        }
        if (pos > start || keep_empty)
            out.push_back(s.substr(start, pos - start));
        start = pos + delim.size();
    }
    return out;
}


std::string utils_parsing::trim(const std::string &str)
{
	std::size_t start = 0;
	while (start < str.length()
		&& std::isspace(static_cast<unsigned char>(str[start])))
		start++;
	std::size_t end = str.length();
	while (end > start && std::isspace(static_cast<unsigned char>(str[end
				- 1])))
		end--;
	return (str.substr(start, end - start));
}


std::vector<std::string> utils_parsing::ft_split(const std::string &str, char delimiter)
{
	std::vector<std::string> result;
	std::string current;
	for (std::size_t i = 0; i < str.length(); ++i)
	{
		if (str[i] == delimiter)
		{
			if (!current.empty())
			{
				result.push_back(trim(current));
				current.clear();
			}
		}
		else
		{
			current += str[i];
		}
	}
	if (!current.empty())
		result.push_back(trim(current));
	return (result);
}

std::vector<std::string> utils_parsing::split_on_substring(const std::string& input, const std::string& delimiter)
{
	std::vector<std::string> result;
	size_t pos = 0;
	size_t prev = 0;

	while ((pos = input.find(delimiter, prev)) != std::string::npos) {
		result.push_back(input.substr(prev, pos - prev));
		prev = pos + delimiter.length();
	}
	result.push_back(input.substr(prev));
	return result;
}

std::vector<std::string> utils_parsing:: multi_split(const std::string& str, const std::string& seps)
{
    std::vector<std::string> result;
    std::string current;

    for (size_t i = 0; i < str.size(); i++) {
        if (seps.find(str[i]) != std::string::npos) {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current += str[i];
        }
    }
    if (!current.empty())
        result.push_back(current);
    return result;
}

std::string utils_parsing::to_lower(const std::string &str)
{
	std::string result;
	for (std::size_t i = 0; i < str.length(); ++i)
		result += std::tolower(static_cast<unsigned char>(str[i]));
	return (result);
}

// GET /search?q=Jean%20Dupont&ville=St%20Ouen HTTP/1.1
// q     → "Jean Dupont"
// ville → "St Ouen"


std::string utils_parsing::decode_url(const std::string& url_str)
{
	std::string result;
	for(std::size_t i = 0; i < url_str.length(); ++i)
	{
		if(url_str[i] == '%' && i + 2 < url_str.length())
		{
			char hex1 = url_str[i + 1];
			char hex2 = url_str[i + 2];
			if(isxdigit(hex1) && isxdigit(hex2))
			{
				std::string hex_str = url_str.substr(i + 1, 2);
				int hex_value;
				std::istringstream(hex_str) >> std::hex >> hex_value;
				result += static_cast<char>(hex_value);
				result += static_cast<char>(hex_value);
				i += 2;
			}
			else
			{
				result += '%';
			}
		}
		else if(url_str[i] == '+')
		{
			result += ' ';
		}
		else
		{
			result += url_str[i];
		}
	}
	return(result);
}


  bool utils_parsing::is_alphabetic(const std::string& str)
	{
		if (str.empty())
			return false;
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (!std::isdigit(str[i]))
				return false;
		}
		return true;
	}

std::string utils_parsing::remove_all_r(const std::string& s)
{
    std::string result;
    std::cout << GREEN << "remove_all deb = " << s << RESET << std::endl;
    for (size_t i = 0; i < s.length(); ++i)
    {
        if (s[i] != '\r' && s[i] != '\t' && s[i] != '\v' && s[i] != '\f')
            result += s[i];
    }
    std::cout << GREEN << "remove_all end = " << result << RESET << std::endl;
    return result;
}
