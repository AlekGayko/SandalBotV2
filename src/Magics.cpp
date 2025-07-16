#include "Magics.h"

namespace SandalBot::BitMagics {

	// Move Hashtables
	std::unique_ptr<Bitboard[]> orthogonalMoves[SQUARES_NB];
	std::unique_ptr<Bitboard[]> diagonalMoves[SQUARES_NB];

	// Initialise hashmaps but creating hashmap for each square of correct size
	void BitMagics::initMagics() {
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			// Size is one greater than max index
			orthogonalMoves[square] = std::make_unique<Bitboard[]>(maxOrthogonalIndexes[square] + 1);
			diagonalMoves[square] = std::make_unique<Bitboard[]>(maxDiagonalIndexes[square] + 1);;
		}
	}

	// Inserts orthogonal movement bitboards for each blocker
	void BitMagics::addOrthogonalMoves(Square square, std::vector<Bitboard>& blockers, std::vector<Bitboard>& movementBoards) {
		for (std::size_t i = 0; i < blockers.size(); i++) {
			orthogonalMoves[square].get()[getOrthIndex(blockers[i], square)] = movementBoards[i];
		}
	}

	// Inserts diagonal movement bitboards for each blocker
	void BitMagics::addDiagonalMoves(Square square, std::vector<Bitboard>& blockers, std::vector<Bitboard>& movementBoards) {
		for (std::size_t i = 0; i < blockers.size(); i++) {
			diagonalMoves[square].get()[getDiagIndex(blockers[i], square)] = movementBoards[i];
		}
	}

}