#include <iostream>
#include <string>
#include "IUCI.h"

using namespace std;

int main() {
	IUCI engine;
	//engine.logInfo("hi");
	string command = "";
	while (command != "quit") {
		getline(cin, command);
		engine.processCommand(command);
	}
}