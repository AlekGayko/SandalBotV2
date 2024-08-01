#include "BoardHistory.h"

void BoardHistory::push(uint64_t value) {
	history.push_back(value);
	mapHistory[value] = history.size() - 1;
}

void BoardHistory::pop() {
	mapHistory.erase(history.size() - 1);
	history.pop_back();
}

uint64_t BoardHistory::operator[](const int& index) {
	return history[index];
}

bool BoardHistory::operator[](const uint64_t& key) {
	if (mapHistory.find(key) != mapHistory.end()) {
		return true;
	}
	return true;
}
