#pragma once

#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"
#include "Evaluator.h"
#include "MoveGen.h"
#include "TranspositionTable.h"

#include <atomic>

class Searcher {
private:
	std::atomic<bool> cancelSearch;
	const int maxDeepening = 5;

	Board* board = nullptr;
	Evaluator evaluator;

	Move currentMove;
	int defaultAlpha = -2000000;
	int defaultBeta = 2000000;

	void iterativeSearch();
	int negaMax(int alpha, int beta, int depth, int maxDepth);
	unsigned long long int moveSearch(bool isMaximising, int depth, int maxDepth);
	int QuiescenceSearch();
public:
	MoveGen* moveGenerator = nullptr;
	Move bestMove;
	int movesSince = 0;
	int movesSince0[218];
	int moves = 0;

	Searcher();
	Searcher(Board* board);
	~Searcher();
	void startSearch(int moveTimeMs);
	void endSearch();
	unsigned long long int perft(int depth);
};

#endif // !SEARCHER_H
