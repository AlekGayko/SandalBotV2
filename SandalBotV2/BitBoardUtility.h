#pragma once

#ifndef BITBOARDUTILITY_H
#define BITBOARDUTILITY_H

#define ulong unsigned long long int

class BitBoardUtility {
public:
	static void printBB(ulong bitboard);
	static ulong getBit(ulong bitboard, int index);
	static void setBit(ulong bitboard, int index);

};

#endif // !BITBOARDUTILITY_H
