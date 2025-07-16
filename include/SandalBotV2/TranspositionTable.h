#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "Move.h"
#include "Types.h"
#include "ZobristHash.h"

#include <limits>

namespace SandalBot {

	// TranspositionTable inherits from ZobristHash to apply zobrist hashing techniques
	// for a hashtable of previously visited positions. Using hashes as indexes, positions'
	// evaluation, bestmove, and other information can be stored to avoid recomputation - 
	// drastically reduced search tree sizes in repetitive positions
	class TranspositionTable {
	public:
		// Sentinel value for evaluation
		static constexpr int notFound{ std::numeric_limits<int>::min() };
		// Node types
		static constexpr uint8_t exact{ 0 };
		static constexpr uint8_t lowerBound{ 1 };
		static constexpr uint8_t upperBound{ 2 };
		// Size of table and number of slots filled
		std::size_t size{};
		std::size_t slotsFilled{};

		TranspositionTable(int sizeMB = defaultSizeMB);
		~TranspositionTable() { delete[] table; }
		Move getBestMove(HashKey hashKey);
		int getDepth(HashKey hashKey);
		void store(int eval, int16_t remainingDepth, int16_t currentDepth, uint8_t nodeType, Move move, HashKey hashKey);
		int lookup(int16_t remainingDepth, int16_t currentDepth, int alpha, int beta, HashKey hashKey);
		void clear();
		int retrieveMateScore(int eval, int16_t currentDepth);
		int storeMateScore(int eval, int16_t currentDepth);
	private:
		// Hash table entry, storing positional information
		struct Entry {
			HashKey hash{ 0ULL };
			int eval{ 0 };
			int16_t depth{ std::numeric_limits<int16_t>::max() };
			uint8_t nodeType{}; // Determines whether node was an exact, upper, or lower bound of evaluation
			Move move{};
			Entry() {}
			Entry(HashKey hash, int eval, uint16_t depth, uint8_t nodeType, Move move) 
				: hash(hash), eval(eval), depth(depth), nodeType(nodeType), move(move) {}
			Entry(Entry&& other) noexcept
				: hash(other.hash), eval(other.eval), depth(other.depth), 
				nodeType(other.nodeType), move(other.move) {}

			Entry& operator=(const Entry& other) {
				this->hash = other.hash;
				this->eval = other.eval;
				this->depth = other.depth;
				this->nodeType = other.nodeType;
				this->move = other.move;

				return *this;
			}
			Entry& operator=(Entry&& other) noexcept {
				this->hash = other.hash;
				this->eval = other.eval;
				this->depth = other.depth;
				this->nodeType = other.nodeType;
				this->move = other.move;

				return *this;
			}
		};
		
		static constexpr std::size_t defaultSizeMB = 128; // Default size of table in MB
		Entry* table{ nullptr };
		Move nullMove{};

		std::size_t getIndex(HashKey hash) const { return hash % size; }
	};

}

#endif // !TRANSPOSITIONTABLE_H
