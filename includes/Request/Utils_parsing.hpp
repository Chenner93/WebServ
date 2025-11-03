/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils_parsing.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thbasse <thbasse@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 15:04:26 by kahoumou          #+#    #+#             */
/*   Updated: 2025/11/03 15:10:14 by thbasse          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <map>
#include <set>
#include <algorithm>
#include <string>

#define RESET	"\033[0m"
#define RED		"\033[31m"
#define GREEN	"\033[32m"
#define YELLOW	"\033[33m"
#define BLUE	"\033[34m"
#define MAGENTA	"\033[35m"
#define CYAN	"\033[36m"

namespace utils_parsing
{
	std::string trim(const std::string &str);
	std::vector<std::string>
	ft_split(const std::string &str, char delimiter);
	std::vector<std::string> multi_split(const std::string& str, const std::string& seps);
	std::string to_lower(const std::string &str);
	std::vector<std::string>
	split_on_substr(const std::string& s, const std::string& delim, bool keep_empty = true);
	std::string decode_url(const std::string& str);
	bool is_alphabetic(const std::string& str);
	std::string remove_all_r(const std::string& s);
	void clean_chunk_lines(std::vector<std::string>& lines);
	std::vector<std::string> split_on_substring(const std::string& input, const std::string& delimiter);
};
