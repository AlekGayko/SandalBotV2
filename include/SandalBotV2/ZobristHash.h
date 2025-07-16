#ifndef ZOBRISTHASH_H
#define ZOBRISTHASH_H

#include "Types.h"

namespace SandalBot {

	class Board;

	// Zobrist Hash implements zobrist hashing of board position. 
	// Represents board position in 64 bit unsigned number
	// by XOR'ing several state factors, associative property of XOR
	// allows progressive changes to hash and ability to revert changes.
	namespace ZobristHash {

		// Hash values of each piece for both colors for every square
		extern HashKey pieceHashes[2][7][64];
		extern HashKey enPassantHash[64]; // Hash values for each en passant target square
		extern HashKey castlingRightsHash[17]; // Hashes for each castling right
		extern HashKey whiteMoveHash; // Hash for sides turn

		void initHashes();

		HashKey hashBoard(Board* board);
	};

}

#endif // !ZOBRISTHASH_H
