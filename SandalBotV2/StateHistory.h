#ifndef STATEHISTORY_H
#define STATEHISTORY_H

#include "BoardState.h"

namespace SandalBot {

	class StateHistory {
	private:
		const size_t defaultSize{ 1024 };
		size_t size{ 0 };
		size_t allocatedSize{ 0 };
		BoardState* history{ nullptr };
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
