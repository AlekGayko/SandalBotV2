#include "TranspositionTable.h"

TranspositionTable::TranspositionTable() {
	this->size = 0;
}

TranspositionTable::TranspositionTable(Board* board, int sizeMB) : ZobristHash(board) {
	this->board = board;
	this->size = (sizeMB / sizeof(Entry)) * 1000000;
	//table = new Entry[size];
}

TranspositionTable::~TranspositionTable() {
	if (table != nullptr) delete[] table;
}

void TranspositionTable::store(int eval, char depth) {
	u64 hashKey = hashBoard();
	int index = hashKey % size;

	if (table[index].hash == hashKey && table[index].depth > depth) return;

	table[index] = Entry(hashKey, eval, depth);
}

int TranspositionTable::lookup(int depth) {
	u64 hashKey = hashBoard();
	int index = hashKey % size;

	if (table[index].hash != hashKey || table[index].depth > depth) return notFound;

	return table[index].eval;
}

void TranspositionTable::clear() {
	if (table == nullptr) return;
}
