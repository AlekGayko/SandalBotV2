#include "TestFileReader.h"

#include <fstream>
#include <string>
#include <vector>

std::vector<std::string> TestFileReader::readFile(std::string& fileName) {
	std::string fileDir = std::string(TEST_DATA_DIR) + fileName;
	std::ifstream file(fileDir);

	if (!file.is_open()) {
		throw std::runtime_error("Error Reading Test File at " + fileDir);
	}

	std::vector<std::string> lines;
	std::string line;

	while (getline(file, line)) {
		if (line.size() > 0)
			lines.push_back(line);
	}

	file.close();

	return lines;
}