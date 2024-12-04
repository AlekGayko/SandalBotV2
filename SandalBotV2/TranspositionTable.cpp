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
	table = new Entry[size];
}

TranspositionTable::~TranspositionTable() {
	delete[] table;
}

Move& TranspositionTable::getBestMove() {
	return table[(board->state->zobristHash) % size].move;
}

void TranspositionTable::store(int eval, int remainingDepth, int currentDepth, int nodeType, Move& move, u64 hashKey) {
	size_t index = hashKey % size;

	table[index] = std::move(Entry(hashKey, adjustMateScore(eval, currentDepth), remainingDepth, nodeType, move));
}

int TranspositionTable::lookup(int remainingDepth, int currentDepth, int alpha, int beta, u64 hashKey) {
	size_t index = hashKey % size;
	Entry& entry = table[index];
	if (entry.hash == hashKey && entry.depth >= remainingDepth) {
		int eval = adjustStoredMateScore(entry.eval, currentDepth);
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

int TranspositionTable::adjustStoredMateScore(int eval, int currentDepth) {
	if (abs(eval) >= Evaluator::checkMateScore) {
		int sign = eval >= 0 ? 1 : -1;
		return (eval * sign - currentDepth) * sign;
	}
	return eval;
}

int TranspositionTable::adjustMateScore(int eval, int currentDepth) {
	if (abs(eval) >= Evaluator::checkMateScore) {
		int sign = eval >= 0 ? 1 : -1;
		return (eval * sign + currentDepth) * sign;
	}
	return eval;
}
 