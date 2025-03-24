#include <iostream>
#include <string>
#include "IUCI.h"

int main() {
	SandalBot::IUCI engine;

	std::string command = "";
	while (command != "quit") {
		std::getline(std::cin, command);
		engine.processCommand(command);
	}

}