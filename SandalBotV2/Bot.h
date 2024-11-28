#pragma once

#ifndef BOT_H
#define BOT_H

#include "Board.h"
#include "Searcher.h"
#include "OpeningBook.h"

class Bot {
private:
	Board* board = nullptr;
	OpeningBook* openingBook = nullptr;
	Searcher* searcher = nullptr;
	int validateUserMove(Move moves[218], int startSquare, int targetSquare, int flag);

	const int maxMoveTime = 3000;
public:
	Bot();
	~Bot();

	void setPosition(std::string FEN);
	void makeMove(std::string movestr);
	std::string generateMove(int moveTimeMs);
	void stopSearching();
	uint64_t perft(int depth);
	std::string printBoard();
	int chooseMoveTime(int whiteTimeMs, int blackTimeMs, int whiteIncMs=0, int blackIncMs=0);
};

#endif // !BOT_H
