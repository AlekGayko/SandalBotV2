#include "Evaluator.h"
#include "TranspositionTable.h"

#include <iostream>

using namespace std;

namespace SandalBot {

	TranspositionTable::TranspositionTable(int sizeMB) {
		this->size = (sizeMB * 1024ULL * 1024ULL) / sizeof(Entry);
		this->table = new Entry[size];
		this->slotsFilled = 0ULL;
	}

	TranspositionTable& TranspositionTable::operator=(const TranspositionTable& other) {
		if (this != &other) {
			delete[] table;  // free old memory
			size = other.size;
			slotsFilled = other.slotsFilled;
			table = new Entry[size];
			std::copy(other.table, other.table + size, table);
		}
		return *this;
	}

	// Store position entry
	void TranspositionTable::store(int eval, int16_t depth, int16_t ply, NodeBound nodeType, Move move, HashKey hashKey) {
		size_t index = getIndex(hashKey);
		if (table[index].hash == 0ULL && slotsFilled < size) {
			slotsFilled++; // Update slots filled
		}
		// Move entry into table
		table[index] = Entry(hashKey, evalToTT(eval, ply), depth, nodeType, std::move(move));
	}

	// Clear table
	void TranspositionTable::clear() {
		if (table == nullptr)
			return;
		slotsFilled = 0ULL;
		delete[] table;
		table = new Entry[size];
	}

	// Checkmate score needs to be recalibrated to currentDepth
	int TranspositionTable::ttToEval(int eval, int16_t ply) {
		if (isMateScore(eval)) {
			return eval + eval >= 0 ? -ply : ply;
		}
		return eval;
	}

	// Checkmate score needs to be adjusted relative to ply
	int TranspositionTable::evalToTT(int eval, int16_t ply) {
		if (isMateScore(eval)) {
			return eval + eval >= 0 ? ply : -ply;
		}
		return eval;
	}

}