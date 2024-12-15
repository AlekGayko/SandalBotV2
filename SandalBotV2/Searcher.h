#pragma once

#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"
#include "Evaluator.h"
#include "MoveGen.h"
#include "MoveLine.h"
#include "MoveOrderer.h"
#include "TranspositionTable.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>

class Searcher {
private:
	friend struct searchStatistics;
	struct searchStatistics {
		Move bestMove;
		u64 qNodes;
		u64 nNodes;
		int depth;
		int seldepth;
		int eval;
		u64 duration;

		std::string prepareEval();
		void printIteration();
		void print(Searcher* searcher);
	};
	Move nullMove;

	searchStatistics stats;

	std::atomic<bool> cancelSearch;
	std::atomic<bool> searchCompleted;
	std::mutex searchMutex;
	std::condition_variable searchStop;
	int searchWaitPeriod = 100;
	const int maxDeepening = 256;
	const int reduceExtensionCutoff = 3;
	const int maxExtensions = 16;
	int bestLineSize;

	MoveLine* bestLine;

	Board* board = nullptr;
	
	TranspositionTable* tTable = nullptr;

	Move currentMove;
	int defaultAlpha = std::numeric_limits<int>::min() + 1; // Using min cannot be negated due to two complement range
	int defaultBeta = std::numeric_limits<int>::max();

	void iterativeSearch();
	int negaMax(int alpha, int beta, int depth, int maxDepth, int numExtensions);
	uint64_t moveSearch(bool isMaximising, int depth, int maxDepth);
	int quiescenceSearch(int alpha, int beta, int maxDepth);
	bool worthSearching(Move& move, const bool isCheck, const int numExtensions);
	void moveSleep(int moveTimeMs);
	void generateBestLine(Move bestMove);
	void enactBestLine(Move& move, int depth);
public:
	Evaluator* evaluator = nullptr;
	MoveGen* moveGenerator = nullptr;
	MoveOrderer* orderer = nullptr;
	Move bestMove;
	int movesSince = 0;

	Searcher();
	Searcher(Board* board);
	~Searcher();
	void startSearch(bool isTimed, int moveTimeMs = 0);
	void endSearch();
	int eval();
	uint64_t perft(int depth);
};

#endif // !SEARCHER_H
