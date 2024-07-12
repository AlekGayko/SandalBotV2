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
};

#endif // !STRINGUTIL_H