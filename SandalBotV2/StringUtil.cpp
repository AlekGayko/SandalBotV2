#include "StringUtil.h"

#include <algorithm>
#include <cctype>
#include <sstream>


using namespace std;

std::string StringUtil::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r\f\v");
	size_t end = str.find_last_not_of(" \t\n\r\f\v");

	return (start == std::string::npos || end == std::string::npos)
		? ""
		: str.substr(start, end - start + 1);
}

std::string StringUtil::toLower(const std::string& str) {
	std::string lowerStr = str;
	std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return lowerStr;
}

vector<string> StringUtil::splitString(string& str) {
	vector<string> words;
	stringstream ss(str);
	string word;

	while (ss >> word) {
		words.push_back(word);
	}

	return words;
}

bool StringUtil::contains(const std::string& str, char ch) {
	return str.find(ch) != std::string::npos;
}
