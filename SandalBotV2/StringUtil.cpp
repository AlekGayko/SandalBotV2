#include "StringUtil.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>


using namespace std;

namespace StringUtil {
	std::string StringUtil::trim(const std::string& str) {
		size_t start = str.find_first_not_of(" \t\n\r\f\v");
		size_t end = str.find_last_not_of(" \t\n\r\f\v");

		return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
	}

	std::string StringUtil::toLower(const std::string& str) {
		std::string lowerStr = str;
		std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), [](unsigned char c) { return std::tolower(c); });
		return lowerStr;
	}

	vector<string> StringUtil::splitString(string& str) {
		vector<string> words;
		stringstream ss(str);
		string word;

		while (ss >> word) {
			words.push_back(word);
		}
		if (!words.size()) {
			words.push_back("");
		}

		return words;
	}

	bool StringUtil::contains(const std::string& str, char ch) {
		return str.find(ch) != std::string::npos;
	}

	bool StringUtil::contains(const std::string& str1, const std::string& str2) {
		return str1.find(str2) != std::string::npos;
	}

	int StringUtil::indexOf(const std::string& str1, const std::string& str2) {
		int index = str1.find(str2);
		if (index == std::string::npos) {
			return -1;
		}
		return index;
	}

	bool StringUtil::isDigitString(const std::string& str) {
		if (!str.size()) {
			return false;
		}

		for (char ch : str) {
			if (!std::isdigit(static_cast<unsigned char>(ch))) {
				return false;
			}
		}
		return true;
	}

	string StringUtil::commaSeparator(int integer) {
		string stringInt = to_string(integer);
		size_t size = stringInt.size();
		for (int i = 0; i < size; i++) {
			if (i != 0 && i != size - 1 && (i + 1) % 3 == 0 && stringInt[size - 1 - i] != '-') {
				stringInt.insert(size - 1 - i, 1, ',');
			}
		}
		return stringInt;
	}
}