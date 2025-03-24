#ifndef ZOBRISTHASH_H
#define ZOBRISTHASH_H

#include <cstdint>

namespace SandalBot {

	class Board;

	class ZobristHash {
	protected:
		Board* board{ nullptr };
	public:
		static bool initialised;
		static uint64_t pieceHashes[2][7][64];
		static uint64_t enPassantHash[64];
		static uint64_t castlingRightsHash[17];
		static uint64_t whiteMoveHash;
		ZobristHash();
		ZobristHash(Board* board);
		uint64_t hashBoard();

		static void initHashes();
		static uint64_t hashBoard(Board* board);
	};

}

#endif // !ZOBRISTHASH_H
