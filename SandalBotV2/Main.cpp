#include <iostream>
#include <bitset>
#include <string>
#include <vector>
#include "IUCI.h"

using namespace std;

int main() {
	IUCI engine;

	string command = "";
	while (command != "quit") {
		cin >> command;
		engine.processCommand(command);
	}
	return 0;
}