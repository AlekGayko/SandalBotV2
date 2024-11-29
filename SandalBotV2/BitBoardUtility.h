#pragma once

#ifndef BITBOARDUTILITY_H
#define BITBOARDUTILITY_H

#include <cstdint>

#define u64 uint64_t

class BitBoardUtility {
public:
	static void printBB(u64 bitboard);
	static u64 getBit(u64 bitboard, int index);
	static void setBit(u64& bitboard, int index);
	static void setBit(u64& bitboard, u64& sideBoard, int index);
	static void moveBit(u64& bitboard, int startBit, int targetBit);
	static void moveBit(u64& bitboard, u64& sideBoard, int startBit, int targetBit);
	static void deleteBit(u64& bitboard, int index);
	static void deleteBit(u64& bitboard, u64& sideBoard, int index);
};

#endif // !BITBOARDUTILITY_H
