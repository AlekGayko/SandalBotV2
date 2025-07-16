#ifndef STATEHISTORY_H
#define STATEHISTORY_H

#include "BoardState.h"

namespace SandalBot {

	// StateHistory stores the history of BoardStates for a Board.
	// Utilised for reverting move changes
	class StateHistory {
	public:
		StateHistory();
		~StateHistory();

		void push(const BoardState& state);
		void pop();
		void clear() { size = 0; }
		BoardState& getSecondLast() const { return history[size > 1 ? size - 2 : 0]; }

		BoardState& back() const { return history[size - 1]; }
	private:
		static constexpr std::size_t defaultSize{ 1024 };
		std::size_t size{ 0 };
		std::size_t allocatedSize{ defaultSize };
		BoardState* history{ nullptr }; // Array of states
	};

}

#endif // !STATEHISTORY_H
