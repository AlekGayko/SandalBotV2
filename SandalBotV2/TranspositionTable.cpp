#include "Board.h"
#include "TranspositionTable.h"

#include <iostream>

using namespace std;

TranspositionTable::TranspositionTable() {
	this->size = 0;
}

TranspositionTable::TranspositionTable(Board* board, int sizeMB) : ZobristHash(board) {
	this->board = board;
	this->size = (float(sizeMB) / float(sizeof(Entry))) * 1024 * 1024;
	table = new Entry[size];
	cout << size << endl;
}

TranspositionTable::~TranspositionTable() {
	delete[] table;
}

Move TranspositionTable::getBestMove() {
	return table[(board->state->zobristHash) % size].move;
}

void TranspositionTable::store(int eval, int depth, int nodeType, Move move, u64 hashKey) {
	//if (hashKey == 0ULL) hashKey = hashBoard();
	size_t index = hashKey % size;

	table[index] = Entry(hashKey, eval, depth, nodeType, move);
}

int TranspositionTable::lookup(int depth, int alpha, int beta, u64 hashKey) {
	//if (hashKey == 0ULL) hashKey = hashBoard();
	size_t index = hashKey % size;
	Entry entry = table[index];
	if (entry.hash == hashKey && entry.depth >= depth) {
		if (entry.nodeType == exact) {
			return entry.eval;
		}
		if (entry.nodeType == upperBound && entry.eval <= alpha) {
			return entry.eval;
		}
		if (entry.nodeType == lowerBound && entry.eval >= beta) {
			return entry.eval;
		}
	}

	return notFound;
}

void TranspositionTable::clear() {
	if (table == nullptr) return;
}
