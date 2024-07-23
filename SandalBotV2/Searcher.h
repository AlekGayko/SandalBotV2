#pragma once

#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"
#include "MoveGen.h"
#include "TranspositionTable.h"

class Searcher {
private:
	bool cancelSearch = false;
	const int maxDeepening = 5;

	Board* board = nullptr;

	void iterativeSearch();
	unsigned long long int moveSearch(bool isMaximising, int depth, int maxDepth);
	int QuiescenceSearch();
public:
	MoveGen* moveGenerator = nullptr;
	int movesSince = 0;
	int movesSince0[218];

	Searcher();
	Searcher(Board* board);
	void startSearch();
	void endSearch();
	unsigned long long int perft(int depth);
};

#endif // !SEARCHER_H
