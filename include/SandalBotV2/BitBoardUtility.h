#ifndef BITBOARDUTILITY_H
#define BITBOARDUTILITY_H

#include <cstdint>
#include <iostream>

#if defined(__GNUC__)
	#include <x86intrin.h>
#elif defined(_MSC_VER) 
	#include <intrin.h>
#endif

// BitBoardUtility provides utility functions for bitboards
namespace SandalBot::BitBoardUtility {

	inline uint64_t getBit(uint64_t bitboard, int index) {
		return bitboard & (1ULL << index);
	}
	// Prints the individual bits of the bitboard
	// the printed output is in form of 8x8 board
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
	// 'pops' the least significant bit from a bitboard and returns the index of that bit
	inline int popLSB(uint64_t& bitBoard) {
		// index of LSB
		int trailingZeroes = static_cast<int>(_tzcnt_u64(bitBoard));

		bitBoard &= bitBoard - 1ULL; // Use bit manipulation to remove LSB
		return trailingZeroes;
	}

	inline int LSB(uint64_t bitBoard) {
		return static_cast<int>(_tzcnt_u64(bitBoard));
	}
}

#endif // !BITBOARDUTILITY_H
