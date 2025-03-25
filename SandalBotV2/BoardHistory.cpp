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

	// Standard push function which adds new hash and start indice.
	// Also dynamically adjusts size of arrays if needed
	void BoardHistory::push(uint64_t value, bool reset) {
		// Double size of arrays if memory has run out
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
		// Push new values
		hashHistory[numBoards] = value;
		startSearchIndicies[numBoards + 1] = reset ? numBoards : startSearchIndicies[numBoards];
		numBoards++; // Increment index
	}

	// Returns a boolean, depending on whether a given hash causes a threefold-repetition
	bool BoardHistory::contains(const uint64_t key) const {
		if (numBoards <= 2) return false;

		int start = numBoards - 2;
		int end = startSearchIndicies[numBoards];
		for (int i = start; i >= end; i--) {
			if (hashHistory[i] == key) return true;
		}
		return false;
	}

	// Resets history by resetting index
	void BoardHistory::clear() {
		numBoards = 0;
		startSearchIndicies[0] = 0;
	}

}