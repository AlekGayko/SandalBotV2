#include <iostream>
#include <string>
#include "IUCI.h"
#include "PieceList.h"

using namespace std;

int main() {
	IUCI engine;
	string command = "";
	while (command != "quit") {
		cin >> command;
		engine.processCommand(command);
	}
}