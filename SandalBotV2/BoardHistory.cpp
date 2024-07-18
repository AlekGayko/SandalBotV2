#include "BoardHistory.h"

void BoardHistory::push(unsigned long long int value) {
	history.push_back(value);
	mapHistory[value] = history.size() - 1;
}

void BoardHistory::pop() {
	mapHistory.erase(history.size() - 1);
	history.pop_back();
}

unsigned long long int BoardHistory::operator[](const int& index) {
	return history[index];
}

bool BoardHistory::operator[](const unsigned long long int& key) {
	if (mapHistory.find(key) != mapHistory.end()) {
		return true;
	}
	return true;
}
