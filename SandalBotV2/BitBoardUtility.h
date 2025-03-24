#ifndef BITBOARDUTILITY_H
#define BITBOARDUTILITY_H

#include <cstdint>
#include <iostream>

namespace SandalBot::BitBoardUtility {

	inline uint64_t getBit(uint64_t bitboard, int index) {
		return bitboard & (1ULL << index);
	}

	inline void printBB(uint64_t bitboard) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				int square = i * 8 + j;
				int bit = getBit(bitboard, square) ? 1 : 0;
				std::cout << " " << bit;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	inline void setBit(uint64_t& bitboard, int index) {
		bitboard |= (1ULL << index);
	}

	inline void setBit(uint64_t& bitboard, uint64_t& sideBoard, int index) {
		bitboard |= (1ULL << index);
		sideBoard |= (1ULL << index);
	}

	inline void moveBit(uint64_t& bitboard, int startBit, int targetBit) {
		bitboard &= ~(1ULL << startBit);
		bitboard |= 1ULL << targetBit;
	}

	inline void moveBit(uint64_t& bitboard, uint64_t& sideBoard, int startBit, int targetBit) {
		bitboard &= ~(1ULL << startBit);
		bitboard |= 1ULL << targetBit;

		sideBoard &= ~(1ULL << startBit);
		sideBoard |= 1ULL << targetBit;
	}

	inline void deleteBit(uint64_t& bitboard, int index) {
		bitboard &= ~(1ULL << index);
	}

	inline void deleteBit(uint64_t& bitboard, uint64_t& sideBoard, int index) {
		bitboard &= ~(1ULL << index);
		sideBoard &= ~(1ULL << index);
	}

	inline int popLSB(uint64_t& bitBoard) {
		int trailingZeroes = static_cast<int>(_tzcnt_u64(bitBoard));

		bitBoard &= bitBoard - 1ULL;
		return trailingZeroes;
	}

}

#endif // !BITBOARDUTILITY_H
