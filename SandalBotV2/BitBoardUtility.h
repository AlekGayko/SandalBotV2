#pragma once

#ifndef BITBOARDUTILITY_H
#define BITBOARDUTILITY_H

#include <cstdint>

#define u64 uint64_t

class BitBoardUtility {
public:
	static void printBB(u64 bitboard);
	static u64 getBit(u64 bitboard, int index);
	static void setBit(u64 bitboard, int index);

};

#endif // !BITBOARDUTILITY_H
