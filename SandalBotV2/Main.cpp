#include <iostream>
#include <string>
#include "IUCI.h"

using namespace std;

int main() {
	//MovePrecomputation preComp = MovePrecomputation();

	IUCI engine;

	string command = "";
	while (command != "quit") {
		getline(cin, command);
		engine.processCommand(command);
	}

}