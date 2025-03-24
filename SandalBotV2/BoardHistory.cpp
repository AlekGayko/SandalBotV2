#include "BoardHistory.h"

#include <iostream>

using namespace std;

namespace SandalBot {

	BoardHistory::BoardHistory() {
		hashHistory = new uint64_t[historySize];
		startSearchIndicies = new int[historySize + 1];

		startSearchIndicies[0] = 0;
	}

	BoardHistory::~BoardHistory() {
		delete[] hashHistory;
		delete[] startSearchIndicies;
	}

	void BoardHistory::push(uint64_t value, bool reset) {
		if (numBoards >= historySize) {
			historySize *= 2;
			uint64_t* newHashHistory = new uint64_t[historySize];
			int* newSearchIndice = new int[historySize + 1];

			for (int i = 0; i < numBoards; i++) {
				newHashHistory[i] = hashHistory[i];
				newSearchIndice[i] = startSearchIndicies[i];
			}
			newSearchIndice[numBoards] = startSearchIndicies[numBoards];

			delete[] hashHistory;
			delete[] startSearchIndicies;

			hashHistory = newHashHistory;
			startSearchIndicies = newSearchIndice;
		}
		hashHistory[numBoards] = value;
		startSearchIndicies[numBoards + 1] = reset ? numBoards : startSearchIndicies[numBoards];
		numBoards++;
	}

	bool BoardHistory::contains(const uint64_t key) const {
		if (numBoards <= 2) return false;

		int start = numBoards - 2;
		int end = startSearchIndicies[numBoards];
		for (int i = start; i >= end; i--) {
			if (hashHistory[i] == key) return true;
		}
		return false;
	}

}