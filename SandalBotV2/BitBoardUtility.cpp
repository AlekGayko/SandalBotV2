#include "BitBoardUtility.h"

#include <iostream>

using namespace std;

namespace BitBoardUtility {
	void BitBoardUtility::printBB(u64 bitboard) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				int square = i * 8 + j;
				int bit = getBit(bitboard, square) ? 1 : 0;
				cout << " " << bit;
			}
			cout << endl;
		}
		cout << endl;
	}

	u64 BitBoardUtility::getBit(u64 bitboard, int index) {
		return bitboard & (1ULL << index);
	}

	void BitBoardUtility::setBit(u64& bitboard, int index) {
		bitboard |= (1ULL << index);
	}

	void BitBoardUtility::setBit(u64& bitboard, u64& sideBoard, int index) {
		bitboard |= (1ULL << index);
		sideBoard |= (1ULL << index);
	}

	void BitBoardUtility::moveBit(u64& bitboard, int startBit, int targetBit) {
		bitboard &= ~(1ULL << startBit);
		bitboard |= 1ULL << targetBit;
	}

	void BitBoardUtility::moveBit(u64& bitboard, u64& sideBoard, int startBit, int targetBit) {
		bitboard &= ~(1ULL << startBit);
		bitboard |= 1ULL << targetBit;

		sideBoard &= ~(1ULL << startBit);
		sideBoard |= 1ULL << targetBit;
	}

	void BitBoardUtility::deleteBit(u64& bitboard, int index) {
		bitboard &= ~(1ULL << index);
	}

	void BitBoardUtility::deleteBit(u64& bitboard, u64& sideBoard, int index) {
		bitboard &= ~(1ULL << index);
		sideBoard &= ~(1ULL << index);
	}

	int BitBoardUtility::popLSB(u64& bitBoard) {
		int trailingZeroes = _tzcnt_u64(bitBoard);

		bitBoard &= bitBoard - 1ULL;
		return trailingZeroes;
	}
}