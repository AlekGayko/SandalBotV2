#include "Bitboards.h"
#include "IUCI.h"
#include "ZobristHash.h"

#include <iostream>
#include <string>

using namespace SandalBot;

int main() {
	ZobristHash::initHashes();
	initBitboards();

	IUCI engine;

	// Repeatedly accept commands
	std::string command = "";
	while (command != "quit") {
		std::getline(std::cin, command);
		engine.processCommand(command); // Let engine process command
	}

	return 0;
}