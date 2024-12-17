#pragma once

#ifndef BITBOARDUTILITY_H
#define BITBOARDUTILITY_H

#include <cstdint>

#define u64 uint64_t

namespace BitBoardUtility {
	void printBB(u64 bitboard);
	u64 getBit(u64 bitboard, int index);
	void setBit(u64& bitboard, int index);
	void setBit(u64& bitboard, u64& sideBoard, int index);
	void moveBit(u64& bitboard, int startBit, int targetBit);
	void moveBit(u64& bitboard, u64& sideBoard, int startBit, int targetBit);
	void deleteBit(u64& bitboard, int index);
	void deleteBit(u64& bitboard, u64& sideBoard, int index);
	int popLSB(u64& bitBoard);
};

#endif // !BITBOARDUTILITY_H
