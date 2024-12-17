#pragma once

#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "ZobristHash.h"
#include "Move.h"

#include <limits>

class TranspositionTable : public ZobristHash {
	struct Entry {
		u64 hash = 0ULL;
		int eval = 0;
		unsigned char depth = std::numeric_limits<unsigned char>::max();
		unsigned char nodeType = 0;
		Move move = Move();
		Entry() {}
		Entry(uint64_t hash, int eval, int depth, int nodeType, Move& move) {
			this->hash = hash;
			this->eval = eval;
			this->depth = depth;
			this->nodeType = nodeType;
			this->move = move;
		}
		Entry& operator=(const Entry& other) {
			this->hash = other.hash;
			this->eval = other.eval;
			this->depth = other.depth;
			this->nodeType = other.nodeType;
			this->move = other.move;

			return *this;
		}
	};
private:
	Entry* table = nullptr;
	u64 sizeMB = 0;
	Move nullMove = Move();
public:
	u64 size;
	u64 slotsFilled;
	static const int notFound = std::numeric_limits<int>::min();
	static const unsigned char exact = 0;
	static const unsigned char lowerBound = 1;
	static const unsigned char upperBound = 2;
	TranspositionTable();
	TranspositionTable(Board* board, int sizeMB);
	~TranspositionTable();
	Move getBestMove(u64 hashKey);
	int getDepth(u64 hashKey);
	void store(int eval, int remainingDepth, int currentDepth, int nodeType, Move& move, u64 hashKey);
	int lookup(int remainingDepth, int currentDepth, int alpha, int beta, u64 hashKey);
	void clear();
	int retrieveMateScore(int eval, int currentDepth);
	int storeMateScore(int eval, int currentDepth);
};

#endif // !TRANSPOSITIONTABLE_H
