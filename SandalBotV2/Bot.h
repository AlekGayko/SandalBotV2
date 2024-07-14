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
public:
	Bot();
	~Bot();

	void makeMove(std::string move);
	void generateMove();
	void stopSearching();
	int perft(int depth);
	std::string printBoard();
};

#endif // !BOT_H
