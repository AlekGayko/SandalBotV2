#ifndef BOARDHISTORY_H
#define BOARDHISTORY_H

#include "Types.h"

namespace SandalBot {

	// BoardHistory stores an array of zobrist hashes, and is able to determine
	// whether there has been a threefold-repetition
	class BoardHistory {
	public:
		BoardHistory();
		~BoardHistory();
		void push(HashKey value, bool reset);
		void pop() { numBoards -= numBoards > 0 ? 1 : 0; }
		bool contains(const HashKey key) const;
		void clear();
	private:
		int historySize{ 1024 }; // Initial size
		HashKey* hashHistory{ nullptr }; // Dynamic array of hashes
		// Dynamic array of indicies, listing the most recent indice 
		// threefold-repetition can be searched from, since certain moves
		// make it impossible to reach the same position again
		int* startSearchIndicies{ nullptr };
		int numBoards{ 0 }; // Current index
	};

}

#endif // !BOARDHISTORY_H
