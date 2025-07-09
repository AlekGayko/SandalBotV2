#ifndef BOARDHISTORY_H
#define BOARDHISTORY_H

#include <iostream>

namespace SandalBot {

	// BoardHistory stores an array of zobrist hashes, and is able to determine
	// whether there has been a threefold-repetition
	class BoardHistory {
	private:
		int historySize{ 1024 }; // Initial size
		uint64_t* hashHistory{ nullptr }; // Dynamic array of hashes
		// Dynamic array of indicies, listing the most recent indice 
		// threefold-repetition can be searched from, since certain moves
		// make it impossible to reach the same position again
		int* startSearchIndicies{ nullptr };
		int numBoards{ 0 }; // Current index
	public:
		BoardHistory();
		~BoardHistory();
		void push(uint64_t value, bool reset);
		void pop() { numBoards -= numBoards > 0 ? 1 : 0; }
		bool contains(const uint64_t key) const;
		void clear();
	};

}

#endif // !BOARDHISTORY_H
