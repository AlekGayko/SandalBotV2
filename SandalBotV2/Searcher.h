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
	int perftMoves = 0;
	Board* board = nullptr;
	MoveGen* moveGenerator = nullptr;

	void iterativeSearch();
	int negaMax(bool isMaximising, int depth, int maxDepth);
	int QuiescenceSearch();
public:
	Searcher();
	Searcher(Board* board);
	void startSearch();
	void endSearch();
	int perft(int depth);
};

#endif // !SEARCHER_H
