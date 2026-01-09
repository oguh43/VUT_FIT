/**
 * @file string_utils.h
 * @brief Declaration of string utility functions
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <algorithm>

/**
 * @brief Trims whitespace from the beginning and end of a string
 * 
 * Removes spaces, tabs, newlines, and other whitespace characters
 * from both the beginning and end of the string.
 * 
 * @param str String to trim
 * @return Trimmed string
 */
inline std::string trim(const std::string& str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        start++;
    }
    
    auto end = str.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    
    return std::string(start, end + 1);
}

#endif // STRING_UTILS_H