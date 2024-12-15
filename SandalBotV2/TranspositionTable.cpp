#include "Board.h"
#include "Evaluator.h"
#include "TranspositionTable.h"

#include <iostream>

using namespace std;

TranspositionTable::TranspositionTable() {
	this->size = 0;
}

TranspositionTable::TranspositionTable(Board* board, int sizeMB) : ZobristHash(board) {
	this->board = board;
	this->size = (float(sizeMB) / float(sizeof(Entry))) * 1024 * 1024;
	this->table = new Entry[size];
	this->slotsFilled = 0ULL;
}

TranspositionTable::~TranspositionTable() {
	delete[] table;
}

Move TranspositionTable::getBestMove(u64 hashKey) {
	if (table[hashKey % size].hash != hashKey)
		return nullMove;

	return table[hashKey % size].move;
}

int TranspositionTable::getDepth(u64 hashKey) {
	if (table[hashKey % size].hash != hashKey)
		return -1;

	return table[hashKey % size].depth;
}

void TranspositionTable::store(int eval, int remainingDepth, int currentDepth, int nodeType, Move& move, u64 hashKey) {
	size_t index = hashKey % size;
	if (table[index].hash == 0ULL) {
		slotsFilled = slotsFilled >= size ? size : slotsFilled + 1;
	}
	table[index] = std::move(Entry(hashKey, storeMateScore(eval, currentDepth), remainingDepth, nodeType, move));
}

int TranspositionTable::lookup(int remainingDepth, int currentDepth, int alpha, int beta, u64 hashKey) {
	size_t index = hashKey % size;
	Entry entry = table[index];
	if (entry.hash == hashKey && entry.depth >= remainingDepth) {
		int eval = retrieveMateScore(entry.eval, currentDepth);
		if (entry.nodeType == exact) {
			return eval;
		}
		if (entry.nodeType == upperBound && eval <= alpha) {
			return eval;
		}
		if (entry.nodeType == lowerBound && eval >= beta) {
			return eval;
		}
	}

	return notFound;
}

void TranspositionTable::clear() {
	if (table == nullptr) return;
}

// Checkmate score needs to be recalibrated to currentDepth
int TranspositionTable::retrieveMateScore(int eval, int currentDepth) {
	if (abs(eval) >= Evaluator::checkMateScore / 2) {
		int sign = eval >= 0 ? 1 : -1;
		return eval - currentDepth * sign;
	}
	return eval;
}

// Checkmate score needs to be adjusted relative to currentDepth
int TranspositionTable::storeMateScore(int eval, int currentDepth) {
	if (abs(eval) >= Evaluator::checkMateScore / 2) {
		int sign = eval >= 0 ? 1 : -1;
		return eval + currentDepth * sign;
	}
	return eval;
}
 