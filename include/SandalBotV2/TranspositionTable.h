#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "Move.h"
#include "Types.h"
#include "ZobristHash.h"

#include <iostream>
#include <limits>

namespace SandalBot {

	// Node types
	enum NodeBound : uint8_t {
		EXACT,
		LOWER_BOUND,
		UPPER_BOUND
	};

	// Hash table entry, storing positional information
	struct Entry {
		HashKey hash{ 0ULL };
		int eval{ 0 };
		int16_t depth{ std::numeric_limits<int16_t>::max() };
		NodeBound bound{}; // Determines whether node was an exact, upper, or lower bound of evaluation
		Move move{};
		Entry() {}
		Entry(HashKey hash, int eval, uint16_t depth, NodeBound bound, Move move)
			: hash(hash), eval(eval), depth(depth), bound(bound), move(move) {
		}
		Entry(Entry&& other) noexcept
			: hash(other.hash), eval(other.eval), depth(other.depth),
			bound(other.bound), move(other.move) {
		}

		Entry& operator=(const Entry& other) {
			this->hash = other.hash;
			this->eval = other.eval;
			this->depth = other.depth;
			this->bound = other.bound;
			this->move = other.move;

			return *this;
		}
		Entry& operator=(Entry&& other) noexcept {
			this->hash = other.hash;
			this->eval = other.eval;
			this->depth = other.depth;
			this->bound = other.bound;
			this->move = other.move;

			return *this;
		}
	};

	// TranspositionTable inherits from ZobristHash to apply zobrist hashing techniques
	// for a hashtable of previously visited positions. Using hashes as indexes, positions'
	// evaluation, bestmove, and other information can be stored to avoid recomputation - 
	// drastically reduced search tree sizes in repetitive positions
	class TranspositionTable {
	public:
		TranspositionTable(int sizeMB = defaultSizeMB);
		~TranspositionTable() { delete[] table; }
		TranspositionTable& operator=(const TranspositionTable& other);

		Entry* getEntry(HashKey hashKey, bool& found) const;
		std::size_t getSize() const { return size; }
		std::size_t getSlotsFilled() const { return slotsFilled; }

		void store(int eval, int16_t depth, int16_t ply, NodeBound nodeType, Move move, HashKey hashKey);
		int lookup(int16_t depth, int16_t ply, int alpha, int beta, HashKey hashKey) const;
		void clear();
		static int ttToEval(int eval, int16_t ply);
		static int evalToTT(int eval, int16_t ply);
	private:		
		static constexpr std::size_t defaultSizeMB = 1024;
		Entry* table{ nullptr };

		std::size_t size{};
		std::size_t slotsFilled{};

		std::size_t getIndex(HashKey hash) const { return hash % size; }
	};

	inline Entry* TranspositionTable::getEntry(HashKey hashKey, bool& found) const {
		Entry* entry = &table[getIndex(hashKey)];
		found = entry->hash == hashKey;
		return entry;
	}

}

#endif // !TRANSPOSITIONTABLE_H
