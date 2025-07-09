#include <iostream>
#include <string>
#include "IUCI.h"

int main() {
	// Instantiate engine
	SandalBot::IUCI engine;

	// Repeatedly accept commands
	std::string command = "";
	while (command != "quit") {
		std::getline(std::cin, command);
		engine.processCommand(command); // Let engine process command
	}

}