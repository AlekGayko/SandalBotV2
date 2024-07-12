#pragma once

#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <vector>

class StringUtil {
public:
	static std::string trim(const std::string& str);
	static std::string toLower(const std::string& str);
	static std::vector<std::string> splitString(std::string& str);
	static bool contains(const std::string& str, char ch);
	static bool contains(const std::string& str1, const std::string& str2);
	static int indexOf(const std::string& str1, const std::string& str2);
	static bool isDigitString(const std::string& str);
};

#endif // !STRINGUTIL_H