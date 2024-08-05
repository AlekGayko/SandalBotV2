#include "BoardHistory.h"

#include <iostream>

using namespace std;

BoardHistory::BoardHistory() {
	mapHistory.reserve(1000);
}

BoardHistory::~BoardHistory() {
	mapHistory.clear();
}

void BoardHistory::push(uint64_t value) {
	mapHistory[value]++;
}

void BoardHistory::pop(uint64_t value) {
	mapHistory.erase(value);
}

bool BoardHistory::operator[](const uint64_t& key) {
	if (mapHistory[key] > 1) {
		//cout << "key: " << key << endl;
		//cout << "history: " << mapHistory[key] << endl;
		return true;
	}
	return false;
}