#pragma once

#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"
#include "TranspositionTable.h"

class Searcher {
private:
	bool cancelSearch = false;
	const int maxDeepening = 5;
	Board* board = nullptr;

	void iterativeSearch();
	int negaMax(int depth, int maxDepth);
	int QuiescenceSearch();
public:
	Searcher();
	Searcher(Board* board);
	void startSearch();
	void endSearch();
};

#endif // !SEARCHER_H
