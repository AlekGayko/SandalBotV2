#pragma once

#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "ZobristHash.h"
#include "Move.h"

class TranspositionTable : public ZobristHash {
	struct Entry {
		uint64_t hash = 0ULL;
		int eval = 0;
		char depth = -1;
		Entry() {}
		Entry(uint64_t hash, int eval, char depth) {
			this->hash = hash;
			this->eval = eval;
			this->depth = depth;
		}
	};
private:
	Entry* table = nullptr;
	int sizeMB = 0;
	int size;
public:
	static const int notFound = -999;
	TranspositionTable();
	TranspositionTable(Board* board, int sizeMB);
	~TranspositionTable();
	void store(int eval, char depth);
	int lookup(int depth);
	void clear();
};

#endif // !TRANSPOSITIONTABLE_H
