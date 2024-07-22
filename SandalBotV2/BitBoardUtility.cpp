#include "BitBoardUtility.h"

#include <iostream>

using namespace std;

void BitBoardUtility::printBB(ulong bitboard) {
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

ulong BitBoardUtility::getBit(ulong bitboard, int index) {
	return bitboard & (1ULL << index);
}

void BitBoardUtility::setBit(ulong bitboard, int index) {
	bitboard |= (1ULL << index);
}
