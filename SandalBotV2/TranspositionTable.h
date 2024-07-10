#pragma once

#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "ZobristHash.h"
#include "Move.h"

class TranspositionTable {
	struct Entry {
		unsigned long long int hash;
		int eval;
		char depth;
		Move move;

		Entry(unsigned long long int hash, int eval, char depth, Move move) {
			this->hash = hash;
			this->eval = eval;
			this->depth = depth;
			//this->move = move;
		}
	};
private:
	Entry* table = nullptr;
	int sizeMB = 0;
public:
	TranspositionTable();
	TranspositionTable(int sizeMB);
};

#endif // !TRANSPOSITIONTABLE_H
