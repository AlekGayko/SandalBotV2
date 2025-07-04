#include "PrecomputedMagics.h"

namespace SandalBot {

	// Initialise hashmaps but creating hashmap for each square of correct size
	PrecomputedMagics::PrecomputedMagics() {
		for (int square = 0; square < 64; square++) {
			// Size is one greater than max index
			orthogonalMoves[square] = new uint64_t[maxOrthogonalIndexes[square] + 1];
			diagonalMoves[square] = new uint64_t[maxDiagonalIndexes[square] + 1];
		}
	}

	// Delete hashmap for each square
	PrecomputedMagics::~PrecomputedMagics() {
		for (int square = 0; square < 64; square++) {
			delete[] orthogonalMoves[square];
			delete[] diagonalMoves[square];
		}
	}

	// Inserts orthogonal movement bitboards for each blocker
	void PrecomputedMagics::addOrthogonalMoves(int square, std::vector<uint64_t>& blockers, std::vector<uint64_t>& movementBoards) {
		for (std::size_t i = 0; i < blockers.size(); i++) {
			orthogonalMoves[square][getOrthIndex(blockers[i], square)] = movementBoards[i];
		}
	}

	// Inserts diagonal movement bitboards for each blocker
	void PrecomputedMagics::addDiagonalMoves(int square, std::vector<uint64_t>& blockers, std::vector<uint64_t>& movementBoards) {
		for (std::size_t i = 0; i < blockers.size(); i++) {
			diagonalMoves[square][getDiagIndex(blockers[i], square)] = movementBoards[i];
		}
	}

}