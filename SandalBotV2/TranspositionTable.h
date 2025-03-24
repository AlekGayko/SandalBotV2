#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "ZobristHash.h"
#include "Move.h"

#include <limits>

namespace SandalBot {

	class TranspositionTable : public ZobristHash {
	private:
		struct Entry {
			uint64_t hash{ 0ULL };
			int eval{ 0 };
			int16_t depth{ std::numeric_limits<int16_t>::max() };
			uint8_t nodeType{};
			Move move{};
			Entry() {}
			Entry(uint64_t hash, int eval, uint8_t depth, uint8_t nodeType, Move move) : hash(hash), eval(eval), depth(depth), 
				nodeType(nodeType), move(move) {}
			Entry(Entry&& other) noexcept
				: hash(other.hash), eval(other.eval), depth(other.depth), 
				nodeType(other.nodeType), move(move) {}

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
	
		Entry* table{ nullptr };
		uint64_t sizeMB{ 0 };
		Move nullMove{};
	public:
		uint64_t size{};
		uint64_t slotsFilled{};
		static constexpr int notFound{ std::numeric_limits<int>::min() };
		static constexpr uint8_t exact{ 0 };
		static constexpr uint8_t lowerBound{ 1 };
		static constexpr uint8_t upperBound{ 2 };

		TranspositionTable() : size(0) {}
		TranspositionTable(Board* board, int sizeMB);
		~TranspositionTable() { delete[] table; }
		Move getBestMove(uint64_t hashKey);
		int getDepth(uint64_t hashKey);
		void store(int eval, int16_t remainingDepth, int16_t currentDepth, uint8_t nodeType, Move move, uint64_t hashKey);
		int lookup(int16_t remainingDepth, int16_t currentDepth, int alpha, int beta, uint64_t hashKey);
		void clear();
		int retrieveMateScore(int eval, int16_t currentDepth);
		int storeMateScore(int eval, int16_t currentDepth);
	};

}

#endif // !TRANSPOSITIONTABLE_H
