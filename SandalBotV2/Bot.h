#ifndef BOT_H
#define BOT_H

#include "Board.h"
#include "Searcher.h"

namespace SandalBot {

	class Bot {
	private:
		const int maxMoveTime{ 3000 };

		Board* board{ nullptr };
		Searcher* searcher{ nullptr };

		int validateUserMove(Move moves[218], int startSquare, int targetSquare, int flag);
	public:
		Bot();
		~Bot();

		void setPosition(std::string FEN);
		void makeMove(std::string movestr);
		std::string generateMove(int moveTimeMs);
		void go();
		int eval();
		void stopSearching();
		uint64_t perft(int depth);
		void printBoard();
		int chooseMoveTime(int whiteTimeMs, int blackTimeMs, int whiteIncMs = 0, int blackIncMs = 0);
		void changeHashSize(int sizeMB);
		void clearHash();
	};

}

#endif // !BOT_H
