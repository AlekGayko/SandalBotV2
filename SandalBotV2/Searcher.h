#pragma once

#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"

class Searcher {
private:
	bool cancelSearch = false;
	Board board;
public:
	Searcher();
	Searcher(Board board);
	void startSearch();
	void iterativeSearch();
	int negaMax();
	int QuiescenceSearch();
	void endSearch();
};

#endif // !SEARCHER_H
