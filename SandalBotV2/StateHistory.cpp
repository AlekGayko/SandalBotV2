#include "stateHistory.h"
#include "BoardState.h"

#include <algorithm>

using namespace std;

namespace SandalBot {

	StateHistory::StateHistory() {
		history = new BoardState[defaultSize];
		allocatedSize = defaultSize;
		for (int i = 0; i < allocatedSize; i++) {
			history[i] = BoardState();
		}
	}

	StateHistory::~StateHistory() {
		delete[] history;
	}

	void StateHistory::push(BoardState& state) {
		if (size >= allocatedSize - 2) {
			BoardState* newHistory = new BoardState[2 * allocatedSize];
			allocatedSize *= 2;

			for (int i = 0; i < size; i++) {
				newHistory[i] = history[i];
			}
			delete[] history;
			history = newHistory;
		}

		history[size++] = std::move(state);

	}

	void StateHistory::pop() {
		if (size <= 1)
			size = 0;
		else
			size--;
	}



	BoardState& StateHistory::getSecondLast() {
		return history[size > 1 ? size - 2 : 0];
	}

	BoardState& StateHistory::back() {
		return history[size - 1];
	}

}