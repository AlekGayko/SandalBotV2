#pragma once

#ifndef ZOBRISTHASH_H
#define ZOBRISTHASH_H

#include <cstdint>

#define u64 uint64_t

class Board;

class ZobristHash {
protected:
	Board* board = nullptr;
public:
	static bool initialised;
	static u64 pieceHashes[2][7][64];
	static u64 enPassantHash[64];
	static u64 castlingRightsHash[17];
	static u64 whiteMoveHash;
	ZobristHash();
	ZobristHash(Board* board);
	void initHashes();
	u64 hashBoard();
};

#endif // !ZOBRISTHASH_H
