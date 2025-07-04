#include "StateHistory.h"
#include "BoardState.h"

#include <algorithm>

using namespace std;

namespace SandalBot {

	StateHistory::StateHistory() {
		history = new BoardState[defaultSize];
		std::fill(history, history + allocatedSize, BoardState());
	}

	StateHistory::~StateHistory() {
		delete[] history;
	}

	// Push new state to history, dynamically adjusts array if memory exceeded
	void StateHistory::push(BoardState& state) {
		// If memory exceeded, double array size and copy elements over
		if (size >= allocatedSize - 2) {
			BoardState* newHistory = new BoardState[2 * allocatedSize];
			allocatedSize *= 2;

			std::copy(newHistory, newHistory + size, history);

			delete[] history;
			history = newHistory;
		}

		history[size++] = std::move(state);
	}

	// Reduces size of history to rollback history
	void StateHistory::pop() {
		if (size <= 1)
			size = 0;
		else
			--size;
	}

	BoardState& StateHistory::getSecondLast() {
		return history[size > 1 ? size - 2 : 0];
	}

	BoardState& StateHistory::back() {
		return history[size - 1];
	}

}