#include "PrecomputedMagics.h"

PrecomputedMagics::PrecomputedMagics() {
	for (int square = 0; square < 64; square++) {
		// Size is one greater than max index
		orthogonalMoves[square] = new uint64_t[maxOrthogonalIndexes[square] + 1];
		diagonalMoves[square] = new uint64_t[maxDiagonalIndexes[square] + 1];
	}
}

PrecomputedMagics::~PrecomputedMagics() {
	for (int square = 0; square < 64; square++) {
		delete[] orthogonalMoves[square];
		delete[] diagonalMoves[square];
	}
}

void PrecomputedMagics::addOrthogonalMoves(int square, std::vector<uint64_t>& blockers, std::vector<uint64_t>& movementBoards) {
	for (int i = 0; i < blockers.size(); i++) {
		uint64_t index = (blockers[i] * orthogonalMagics[square]) >> orthogonalShifts[square];
		orthogonalMoves[square][index] = movementBoards[i];
	}
}

void PrecomputedMagics::addDiagonalMoves(int square, std::vector<uint64_t>& blockers, std::vector<uint64_t>& movementBoards) {
	for (int i = 0; i < blockers.size(); i++) {
		uint64_t index = (blockers[i] * diagonalMagics[square]) >> diagonalShifts[square];
		diagonalMoves[square][index] = movementBoards[i];
	}
}

uint64_t PrecomputedMagics::getOrthogonalMovement(const int& square, const uint64_t& blockers) {
	return orthogonalMoves[square][(blockers * orthogonalMagics[square]) >> orthogonalShifts[square]];
}

uint64_t PrecomputedMagics::getDiagonalMovement(const int& square, const uint64_t& blockers) {
	return diagonalMoves[square][(blockers * diagonalMagics[square]) >> diagonalShifts[square]];;
}