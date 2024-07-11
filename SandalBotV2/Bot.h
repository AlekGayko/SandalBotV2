#pragma once

#ifndef BOT_H
#define BOT_H

#include "Board.h"
#include "Searcher.h"
#include "OpeningBook.h"

class Bot {
private:
	Board board;
	OpeningBook openingBook;
	Searcher searcher;



public:
	Bot();
	Bot(std::string fen);

	void generateMove();
	void stopSearching();

};

#endif // !BOT_H
