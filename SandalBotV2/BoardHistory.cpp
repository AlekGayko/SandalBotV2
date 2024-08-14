#include "BoardHistory.h"

#include <iostream>

using namespace std;

BoardHistory::BoardHistory() {
	hashHistory = new u64[historySize];
	startSearchIndicies = new int[historySize + 1];

	startSearchIndicies[0] = 0;
}

BoardHistory::~BoardHistory() {
	delete[] hashHistory;
	delete[] startSearchIndicies;
}

void BoardHistory::push(u64 value, bool reset) {
	if (numBoards >= historySize) return;
	hashHistory[numBoards] = value;
	startSearchIndicies[numBoards + 1] = reset ? numBoards : startSearchIndicies[numBoards];
	numBoards++;
}

void BoardHistory::pop() {
	numBoards -= numBoards > 0 ? 1 : 0;
}

bool BoardHistory::contains(const u64& key) {
	int start = numBoards >= 2 ? numBoards - 2 : 0;
	int end = startSearchIndicies[numBoards];
	for (int i = start; i >= end; i--) {
		if (hashHistory[i] == key) return true;
	}
	return false;
}