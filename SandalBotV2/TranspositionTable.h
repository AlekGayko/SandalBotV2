#pragma once

#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "ZobristHash.h"
#include "Move.h"

class TranspositionTable : public ZobristHash {
	struct Entry {
		uint64_t hash = 0ULL;
		int eval = 0;
		int depth = 1000000;
		int nodeType;
		Move move;
		Entry() {}
		Entry(uint64_t hash, int eval, int depth, int nodeType, Move move) {
			this->hash = hash;
			this->eval = eval;
			this->depth = depth;
			this->nodeType = nodeType;
			this->move = move;
		}
	};
private:
	Entry* table = nullptr;
	size_t sizeMB = 0;
	size_t size;
public:
	static const int notFound = -999;
	static const int exact = 0;
	static const int lowerBound = 1;
	static const int upperBound = 2;
	TranspositionTable();
	TranspositionTable(Board* board, int sizeMB);
	~TranspositionTable();
	Move getBestMove();
	void store(int eval, int depth, int nodeType, Move move, u64 hashKey);
	int lookup(int depth, int alpha, int beta, u64 hashKey);
	void clear();
};

#endif // !TRANSPOSITIONTABLE_H
