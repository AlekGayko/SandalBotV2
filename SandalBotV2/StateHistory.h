#ifndef STATEHISTORY_H
#define STATEHISTORY_H

#include "BoardState.h"

namespace SandalBot {

	// StateHistory stores the history of BoardStates for a Board.
	// Utilised for reverting move changes
	class StateHistory {
	private:
		static constexpr size_t defaultSize{ 1024 };
		size_t size{ 0 };
		size_t allocatedSize{ defaultSize };
		BoardState* history{ nullptr }; // Array of states
	public:
		StateHistory();
		~StateHistory();

		void push(BoardState& state);
		void pop();
		void clear() { size = 0; }
		BoardState& getSecondLast();
		BoardState& back();
	};

}

#endif // !STATEHISTORY_H
