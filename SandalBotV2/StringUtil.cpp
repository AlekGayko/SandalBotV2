#include "StringUtil.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>


using namespace std;

namespace SandalBot::StringUtil {

	// Removes leading and ending whitespace from a string
	std::string trim(const std::string& str) {
		// Find start and end index of string that does not contain whitespace
		size_t start = str.find_first_not_of(" \t\n\r\f\v");
		size_t end = str.find_last_not_of(" \t\n\r\f\v");
		
		// If no non-whitespace characters found, return empty string
		if (start == std::string::npos || end == std::string::npos) {
			return "";
		} 
		// Return substring not containing leading whitespace
		else {
			return str.substr(start, end - start + 1);
		}
	}

	// Converts string to all lowercase characters
	std::string toLower(const std::string& str) {
		std::string lowerStr = str;
		// Transform every character to lowercase
		std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), 
			[](unsigned char c) { return std::tolower(c); });
		return lowerStr;
	}

	// Splits string str by whitespace, returns split strings via vector<string>
	vector<string> splitString(std::string_view str) {
		vector<string> words; // Vector for storing strings
		stringstream ss(str.data()); // Init stringstream
		string word;

		while (ss >> word) { // Extract string from stream
			words.push_back(word); // Add to vector
		}
		// If no words stored, add empty string
		if (!words.size()) {
			words.push_back("");
		}

		return words;
	}

	// Returns true if str contains char ch
	bool contains(const std::string& str, char ch) {
		return str.find(ch) != std::string::npos;
	}

	// Returns true if str2 is in str1
	bool contains(const std::string& str1, const std::string& str2) {
		return str1.find(str2) != std::string::npos;
	}

	// Returns index of substring str2 in str1
	int indexOf(const std::string& str1, const std::string& str2) {
		std::size_t index = str1.find(str2);
		if (index == std::string::npos) {
			return -1; // Sentinel value
		}
		return index;
	}

	// Returns true if string str is a number
	bool isDigitString(const std::string& str) {
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

	// Returns string representation of integer, with commas separating every 3 digits
	string commaSeparator(int integer) {
		std::string stringInt = to_string(integer);
		std::size_t size = stringInt.size();
		for (std::size_t i = 0; i < size; i++) {
			if (i != 0 && i != size - 1 && (i + 1) % 3 == 0 && stringInt[size - 1 - i] != '-') {
				stringInt.insert(size - 1 - i, 1, ',');
			}
		}
		return stringInt;
	}

}