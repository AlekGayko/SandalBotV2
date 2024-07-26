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
public:
	Bot();
	~Bot();

	void setPosition(std::string FEN);
	void makeMove(std::string movestr);
	void generateMove(int moveTimeMs);
	void stopSearching();
	unsigned long long int perft(int depth);
	std::string printBoard();
};

#endif // !BOT_H
