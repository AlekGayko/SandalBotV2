#include "Init.h"
#include "IUCI.h"

#include <iostream>
#include <string>

using namespace SandalBot;

int main() {
	initGlobals();
	
	IUCI engine;

	// Repeatedly accept commands
	std::string command = "";
	while (command != "quit") {
		std::getline(std::cin, command);
		engine.processCommand(command); // Let engine process command
	}

	return 0;
}