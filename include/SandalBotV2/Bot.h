#ifndef BOT_H
#define BOT_H

#include "Board.h"
#include "Searcher.h"

#include <optional>
#include <string_view>

namespace SandalBot {

	// Bot class represents SandalBot, it encapsulates a board for game state,
	// and a Searcher for generating optimal moves. Provides an interface for making 
	// moves, and providing static evaluation.
	class Bot {
	public:
		Bot();
		~Bot();

		void setPosition(std::string_view FEN);
		void makeMove(std::string movestr);
		std::optional<Move> generateMove(int moveTimeMs);
		std::optional<Move> go();
		int eval();
		void stopSearching();
		uint64_t perft(int depth);
		void printBoard();
		int chooseMoveTime(int whiteTimeMs, int blackTimeMs, int whiteIncMs = 0, int blackIncMs = 0);
		void changeHashSize(int sizeMB);
		void clearHash();
	private:
		const int maxMoveTime{ 3000 }; // Maximum move time

		Board* board{ nullptr };
		Searcher* searcher{ nullptr };

		Move validateUserMove(Square from, Square to, Move::Flag flag);
	};

}

#endif // !BOT_H
