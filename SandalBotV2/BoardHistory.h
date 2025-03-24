#ifndef BOARDHISTORY_H
#define BOARDHISTORY_H

#include <iostream>

namespace SandalBot {

	class BoardHistory {
	private:
		int historySize{ 1024 };
		uint64_t* hashHistory{ nullptr };
		int* startSearchIndicies{ nullptr };
		int numBoards{ 0 };
	public:
		BoardHistory();
		~BoardHistory();
		void push(uint64_t value, bool reset);
		void pop() { numBoards -= numBoards > 0 ? 1 : 0; }
		bool contains(const uint64_t key) const;
	};

}

#endif // !BOARDHISTORY_H
