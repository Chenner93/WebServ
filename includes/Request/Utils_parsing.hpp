/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils_parsing.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kahoumou <kahoumou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 15:04:26 by kahoumou          #+#    #+#             */
/*   Updated: 2025/09/11 14:09:35 by kahoumou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include <cctype>
# include <iostream>
# include <sstream>
# include <string>
# include <vector>
#include <stdexcept>
#include <vector>
#include<iomanip>
#include <map>
#include <set>



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
};
