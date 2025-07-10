#ifndef ZOBRISTHASH_H
#define ZOBRISTHASH_H

#include <cstdint>

namespace SandalBot {

	class Board;

	// Base class Zobrist Hash implements zobrist hashing of 
	// board position. Represents board position in 64 bit unsigned number
	// by XOR'ing several state factors, associative property of XOR
	// allows progressive changes to hash and ability to revert changes.
	class ZobristHash {
	public:
		static uint64_t pieceHashes[2][7][64];
		static uint64_t enPassantHash[64];
		static uint64_t castlingRightsHash[17];
		static uint64_t whiteMoveHash;

		static void initHashes();
		static uint64_t hashBoard(Board* board);

		ZobristHash();
	protected:
		static bool initialised;
	};

}

#endif // !ZOBRISTHASH_H
