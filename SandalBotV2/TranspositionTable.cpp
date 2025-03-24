#include "Board.h"
#include "Evaluator.h"
#include "TranspositionTable.h"

#include <iostream>

using namespace std;

namespace SandalBot {

	TranspositionTable::TranspositionTable(Board* board, int sizeMB) : ZobristHash(board) {
		this->board = board;
		this->size = (float(sizeMB) / float(sizeof(Entry))) * 1024 * 1024;
		this->table = new Entry[size];
		this->slotsFilled = 0ULL;
	}

	Move TranspositionTable::getBestMove(uint64_t hashKey) {
		if (table[hashKey % size].hash != hashKey)
			return std::move(nullMove);

		return std::move(table[hashKey % size].move);
	}

	int TranspositionTable::getDepth(uint64_t hashKey) {
		if (table[hashKey % size].hash != hashKey)
			return -1;

		return table[hashKey % size].depth;
	}

	void TranspositionTable::store(int eval, int16_t remainingDepth, int16_t currentDepth, uint8_t nodeType, Move move, uint64_t hashKey) {
		size_t index = hashKey % size;
		if (table[index].hash == 0ULL && slotsFilled < size) {
			slotsFilled++;
		}
		table[index] = Entry(hashKey, storeMateScore(eval, currentDepth), remainingDepth, nodeType, std::move(move));
	}

	int TranspositionTable::lookup(int16_t remainingDepth, int16_t currentDepth, int alpha, int beta, uint64_t hashKey) {
		size_t index = hashKey % size;
		Entry& entry = table[index];
		if (entry.hash == hashKey && (entry.depth >= remainingDepth || Evaluator::isMateScore(entry.eval))) {
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
		if (table == nullptr)
			return;
		slotsFilled = 0ULL;
		delete[] table;
		table = new Entry[size];
	}

	// Checkmate score needs to be recalibrated to currentDepth
	int TranspositionTable::retrieveMateScore(int eval, int16_t currentDepth) {
		if (Evaluator::isMateScore(eval)) {
			int sign = eval >= 0 ? 1 : -1;
			return eval - currentDepth * sign;
		}
		return eval;
	}

	// Checkmate score needs to be adjusted relative to currentDepth
	int TranspositionTable::storeMateScore(int eval, int16_t currentDepth) {
		if (Evaluator::isMateScore(eval)) {
			int sign = eval >= 0 ? 1 : -1;
			return eval + currentDepth * sign;
		}
		return eval;
	}

}