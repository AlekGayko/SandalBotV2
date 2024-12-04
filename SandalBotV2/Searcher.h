#pragma once

#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"
#include "Evaluator.h"
#include "MoveGen.h"
#include "MoveOrderer.h"
#include "TranspositionTable.h"

#include <atomic>
#include <limits>

class Searcher {
private:
	struct searchStatistics {
		u64 qNodes;
		u64 nNodes;
		u64 cutoffs;
		u64 transpositions;
		u64 checkmates;
		u64 stalemates;
		u64 repetitions;
		u64 fiftyMoveDraws;
		int maxDepth;
		int eval;
		float duration;
		searchStatistics() {
			qNodes = 0ULL;
			nNodes = 0ULL;
			cutoffs = 0ULL;
			transpositions = 0ULL;
			checkmates = 0ULL;
			stalemates = 0ULL;
			repetitions = 0ULL;
			fiftyMoveDraws = 0ULL;
			maxDepth = 0;
			eval = 0;
			duration = 0.0f;
		}
	};
	Move nullMove;

	searchStatistics stats;

	std::atomic<bool> cancelSearch;
	const int maxDeepening = 256;
	const int reduceExtensionCutoff = 5;

	Board* board = nullptr;
	Evaluator evaluator;
	TranspositionTable* tTable = nullptr;

	Move currentMove;
	int defaultAlpha = std::numeric_limits<int>::min() + 1; // Using min cannot be negated due to two complement range
	int defaultBeta = std::numeric_limits<int>::max();

	void iterativeSearch();
	int negaMax(int alpha, int beta, int depth, int maxDepth, int numExtensions);
	uint64_t moveSearch(bool isMaximising, int depth, int maxDepth);
	int QuiescenceSearch(int alpha, int beta, int maxDepth);
	bool worthSearching(const Move& move);
public:
	MoveGen* moveGenerator = nullptr;
	MoveOrderer* orderer = nullptr;
	Move bestMove;
	int movesSince = 0;
	int movesSince0[218];
	int moves = 0;

	Searcher();
	Searcher(Board* board);
	~Searcher();
	void startSearch(int moveTimeMs);
	void endSearch();
	uint64_t perft(int depth);
};

#endif // !SEARCHER_H
