#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <string_view>
#include <vector>

// StringUtil is a namespace for utility functions regarding strings
namespace SandalBot::StringUtil {

	std::string trim(std::string_view str);
	std::string toLower(std::string_view str);
	std::vector<std::string> splitString(std::string_view str);
	bool contains(std::string_view str, char ch);
	bool contains(std::string_view str1, std::string_view str2);
	int indexOf(std::string_view str1, std::string_view str2);
	bool isDigitString(std::string_view str);
	std::string commaSeparator(int integer);

};

#endif // !STRINGUTIL_H