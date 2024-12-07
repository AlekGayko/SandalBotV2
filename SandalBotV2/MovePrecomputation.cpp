#include "MovePrecomputation.h"

#include "BitBoardUtility.h"
#include "Precomputedmagics.h"

#include <cmath>
#include <iostream>
#include <limits>

using namespace std;

MovePrecomputation::MovePrecomputation() {
	magics = new PrecomputedMagics();
	for (int square = 0; square < 64; square++) {
		int row, col, top, left, right, bottom;
		row = square / 8;
		col = square % 8;
		top = row + 1;
		bottom = 8 - row;
		left = col + 1;
		right = 8 - col;
		directionDistances[square] = dirDist(top, left, right, bottom);
	}
	initMasks();
	precomputeMoves();
}

MovePrecomputation::~MovePrecomputation() {
	delete magics;
}

void MovePrecomputation::initMasks() {
	// Blocker Masks
	for (int square = 0; square < 64; square++) {
		createOrthogonalMask(square);
		createDiagonalMask(square);
	}

	// Orthogonal masks
	for (int i = 0; i < 8; i++) {
		rowMasks[i] = rowMask << i * 8;
		columnMasks[i] = columnMask << i;
	}

	// Diagonal masks
	for (int i = 0; i < 15; i++) {
		initForwardMask(i);
		initBackwardMask(i);
	}

	initDirectionMasks();
	initPassedPawnMasks();
	initIslandMasks();
	initShieldMasks();
}

// Init 45 degree diagonal masks
void MovePrecomputation::initForwardMask(int constant) {
	uint64_t mask = 0ULL;
	int col = 0;
	int row = constant - col;

	while (col < 8) {
		if (row >= 0 && row < 8) {
			mask |= 1ULL << (row * 8 + col);
		}
		col++;
		row = constant - col;
	}

	forwardDiagonalMasks[constant] = mask;
}

// Init -45 degree diagonal masks
void MovePrecomputation::initBackwardMask(int constant) {
	constant = constant - 7;
	uint64_t mask = 0ULL;
	int col = 0;
	int row = constant + col;

	while (col < 8) {
		if (row >= 0 && row < 8) {
			mask |= 1ULL << (row * 8 + col);
		}
		col++;
		row = constant + col;
	}

	backwardDiagonalMasks[constant + 7] = mask;
}

void MovePrecomputation::initDirectionMasks() {
	for (int square = 0; square < 64; square++) {
		// Calculate direction masks
		for (int dirIndex = startOrthogonal; dirIndex < endDiagonal; dirIndex++) {
			directionMasks[square * 8 + dirIndex] = 0ULL;
			int distance = directionDistances[square].direction[dirIndex];
			for (int it = 1; it < distance; it++) {
				int newSquare = square + slideDirections[dirIndex] * it;
				directionMasks[square * 8 + dirIndex] |= 1ULL << newSquare;
			}
		}
	}

	for (int diff = -64; diff < 64; diff++) {
		int offset = diff < 0 ? 0 : 2;
		if (diff % 9 == 0) {
			differenceDivisibles[64 + diff] = 4 + offset;
		} else if (diff % 7 == 0) {
			differenceDivisibles[64 + diff] = 5 + offset;
		}
	}

}

void MovePrecomputation::initPassedPawnMasks() {
	for (int square = 0; square < 64; square++) {
		uint64_t whiteFrontMask = numeric_limits<uint64_t>::max() >> min(((7 - square / 8) + 1), 7) * 8;
		uint64_t blackFrontMask = numeric_limits<uint64_t>::max() << min((square / 8 + 1), 7) * 8;

		uint64_t columnMask = columnMasks[square % 8];
		columnMask |= columnMasks[min((square % 8) + 1, 7)];
		columnMask |= columnMasks[max((square % 8) - 1, 0)];

		whitePassedPawnMasks[square] = whiteFrontMask & columnMask;
		blackPassedPawnMasks[square] = blackFrontMask & columnMask;
	}
}

void MovePrecomputation::initIslandMasks() {
	for (int col = 0; col < 8; col++) {
		uint64_t mask = 0ULL;
		if (directionDistances[col].right > 1) {
			mask |= columnMasks[col + 1];
		}
		if (directionDistances[col].left > 1) {
			mask |= columnMasks[col - 1];
		}

		pawnIslandMasks[col] = mask;
	}
}

void MovePrecomputation::initShieldMasks() {
	for (int square = 0; square < 64; square++) {
		uint64_t whiteMask = (numeric_limits<uint64_t>::max() << 6 * 8) >> min(((7 - square / 8) + 1), 7) * 8;
		uint64_t blackMask = (numeric_limits<uint64_t>::max() >> 6 * 8) << min((square / 8 + 1), 7) * 8;

		uint64_t colMask = 0ULL;
		colMask |= columnMasks[square % 8];

		colMask |= columnMasks[min(1 + square % 8, 7)];

		colMask |= columnMasks[max(-1 + square % 8, 0)];

		whiteMask &= colMask;
		blackMask &= colMask;
		
		whitePawnShieldMask[square] = whiteMask;
		blackPawnShieldMask[square] = blackMask;
	}
}

void MovePrecomputation::initDistances() {
	for (int square1 = 0; square1 < 64; square1++) {
		for (int square2 = 0; square2 < 64; square2++) {
			int file1, file2, rank1, rank2;
			int rankDistance, fileDistance;
			file1 = square1 % 8;
			file2 = square2 % 8;
			rank1 = square1 / 8;
			rank2 = square2 / 8;
			rankDistance = abs(rank2 - rank1);
			fileDistance = abs(file2 - file1);
			distances[square1][square2] = max(rankDistance, fileDistance);
		}
	}
}

void MovePrecomputation::precomputeMoves() {
	precomputeOrthogonalMoves();
	precomputeDiagonalMoves();
	precomputeKnightMoves();
	precomputeKingMoves();
	precomputePawnMoves();
}

void MovePrecomputation::precomputeOrthogonalMoves() {
	int size = 0;
	for (int square = 0; square < 64; square++) {
		vector<uint64_t> blockerConfigs = precomputeOrthogonalMove(square);
		vector<uint64_t> movementBoards;
		movementBoards.reserve(blockerConfigs.size());
		size += blockerConfigs.size();
		//generateMagicNumbers(blockerConfigs);
		for (uint64_t& blockerBoard : blockerConfigs) {
			uint64_t moves = createOrthogonalMovement(square, blockerBoard);
			movementBoards.push_back(moves);
		}
		magics->addOrthogonalMoves(square, blockerConfigs, movementBoards);
	}
}

void MovePrecomputation::precomputeDiagonalMoves() {
	int size = 0;
	for (int square = 0; square < 64; square++) {
		vector<uint64_t> blockerConfigs = precomputeDiagonalMove(square);
		vector<uint64_t> movementBoards;
		movementBoards.reserve(blockerConfigs.size());
		size += blockerConfigs.size();
		//generateMagicNumbers(blockerConfigs, false);
		for (uint64_t& blockerBoard : blockerConfigs) {
			uint64_t moves = createDiagonalMovement(square, blockerBoard);
			movementBoards.push_back(moves);
		}
		magics->addDiagonalMoves(square, blockerConfigs, movementBoards);
	}
}

void MovePrecomputation::precomputeKnightMoves() {
	for (int square = 0; square < 64; square++) {
		knightMoves[square] = 0ULL;
		for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
			if (directionDistances[square].knightSquares[dirIndex]) {
				knightMoves[square] |= 1ULL << (square + knightDirections[dirIndex]);
			}
		}
	}
}

void MovePrecomputation::precomputeKingMoves() {
	for (int square = 0; square < 64; square++) {
		kingMoves[square] = 0ULL;
		// Calculate diagonal moves
		for (int dirIndex = startOrthogonal; dirIndex < endDiagonal; dirIndex++) {
			int distance = directionDistances[square].direction[dirIndex];
			if (distance <= 1)
				continue;

			kingMoves[square] |= 1ULL << (square + slideDirections[dirIndex]);
		}
	}
}

void MovePrecomputation::precomputePawnMoves() {
	for (int square = 0; square < 64; square++) {
		whitePawnAttackMoves[square] = 0ULL;
		blackPawnAttackMoves[square] = 0ULL;
		// White pawns
		if (directionDistances[square].direction[4] > 1) {
			whitePawnAttackMoves[square] |= 1ULL << (square + slideDirections[4]);
		}
		if (directionDistances[square].direction[5] > 1) {
			whitePawnAttackMoves[square] |= 1ULL << (square + slideDirections[5]);
		}
		// Black pawns
		if (directionDistances[square].direction[6] > 1) {
			blackPawnAttackMoves[square] |= 1ULL << (square + slideDirections[6]);
		}
		if (directionDistances[square].direction[7] > 1) {
			blackPawnAttackMoves[square] |= 1ULL << (square + slideDirections[7]);
		}
	}
}

std::vector<uint64_t> MovePrecomputation::precomputeOrthogonalMove(int square) {
	vector<int> moveSquares;
	// Calculate orthogonal moves
	for (int i = 0; i < 64; i++) {
		int row = i / 8;
		int col = i % 8;
		if (i == square) {
			continue;
		} else if ((square / 8 != 0 && row == 0) || (square / 8 != 7 && row == 7)) {
			continue;
		} else if ((square % 8 != 0 && col == 0) || (square % 8 != 7 && col == 7)) {
			continue;
		} else if (square / 8 == i / 8) {
			moveSquares.push_back(i);
		} else if (square % 8 == i % 8) {
			moveSquares.push_back(i);
		}
		
	}
	
	return makeConfigs(moveSquares);
}

std::vector<uint64_t> MovePrecomputation::precomputeDiagonalMove(int square) {
	vector<int> moveSquares;
	// Calculate diagonal moves
	for (int dirIndex = startDiagonal; dirIndex < endDiagonal; dirIndex++) {
		int distance = directionDistances[square].direction[dirIndex];
		for (int it = 1; it < distance; it++) {
			int newSquare = square + slideDirections[dirIndex] * it;
			int row = newSquare / 8;
			int col = newSquare % 8;
			if ((square / 8 != 0 && row == 0) || (square / 8 != 7 && row == 7)) {
				break;
			} else if ((square % 8 != 0 && col == 0) || (square % 8 != 7 && col == 7)) {
				break;
			}
			moveSquares.push_back(newSquare);
		}
	}

	return makeConfigs(moveSquares);
}

std::vector<uint64_t> MovePrecomputation::makeConfigs(std::vector<int>& moveSquares) {
	int n = moveSquares.size();
	int numConfigs = 1 << n; // 2^n
	std::vector<uint64_t> blockerConfigs;
	blockerConfigs.reserve(n);

	for (int i = 0; i < numConfigs; i++)
		blockerConfigs.push_back(0ULL);

	for (int pattern = 0; pattern < numConfigs; pattern++) {
		for (int bitIndex = 0; bitIndex < n; bitIndex++) {
			uint64_t bit = (pattern & (1ULL << bitIndex)) >> bitIndex;
			blockerConfigs[pattern] |= bit << moveSquares[bitIndex];
		}
		
	}

	return blockerConfigs;
}

uint64_t MovePrecomputation::createOrthogonalMask(int square) {
	uint64_t mask = 0ULL;
	mask |= blockerRowMask << (square / 8) * 8;

	mask |= blockerColumnMask << (square % 8);
	mask &= ~(1ULL << square);

	blockerOrthogonalMasks[square] = mask;
	
	return mask;
}

uint64_t MovePrecomputation::createDiagonalMask(int square) {
	uint64_t mask = 0ULL;

	for (int dirIndex = startDiagonal; dirIndex < endDiagonal; dirIndex++) {
		int distance = directionDistances[square].direction[dirIndex];
		for (int it = 1; it < distance; it++) {
			int newSquare = square + slideDirections[dirIndex] * it;
			int row = newSquare / 8;
			int col = newSquare % 8;
			if ((square / 8 != 0 && row == 0) || (square / 8 != 7 && row == 7)) {
				break;
			} else if ((square % 8 != 0 && col == 0) || (square % 8 != 7 && col == 7)) {
				break;
			}
			mask |= 1ULL << newSquare;
		}
	}
	blockerDiagonalMasks[square] = mask;

	return mask;
}

uint64_t MovePrecomputation::createOrthogonalMovement(int square, uint64_t blockerBoard) {
	return createMovement(square, blockerBoard, startOrthogonal, endOrthogonal);
}

uint64_t MovePrecomputation::createDiagonalMovement(int square, uint64_t blockerBoard) {
	return createMovement(square, blockerBoard, startDiagonal, endDiagonal);
}

uint64_t MovePrecomputation::createMovement(int square, uint64_t blockerBoard, int start, int end) {
	uint64_t movementBoard = 0ULL;

	for (int dirIndex = start; dirIndex < end; dirIndex++) {
		int distance = directionDistances[square].direction[dirIndex];
		for (int it = 1; it < distance; it++) {
			int newSquare = square + slideDirections[dirIndex] * it;

			movementBoard |= 1ULL << newSquare;

			if (BitBoardUtility::getBit(blockerBoard, newSquare)) {
				break;
			}
		}
	}

	return movementBoard;
}

uint64_t MovePrecomputation::getForwardMask(const int& square) {
	return forwardDiagonalMasks[(square / 8) + (square % 8)];
}

uint64_t MovePrecomputation::getBackwardMask(const int& square) {
	return backwardDiagonalMasks[7 + (square / 8) - (square % 8)];
}

uint64_t MovePrecomputation::getRowMask(const int& square) {
	return rowMasks[square / 8];
}

uint64_t MovePrecomputation::getColMask(const int& square) {
	return columnMasks[square % 8];
}

uint64_t MovePrecomputation::getBlockerOrthogonalMask(const int& square) {
	return blockerOrthogonalMasks[square];
}

uint64_t MovePrecomputation::getBlockerDiagonalMask(const int& square) {
	return blockerDiagonalMasks[square];
}

uint64_t MovePrecomputation::getOrthMovementBoard(const int& square, const uint64_t& blockerBoard) {
	return magics->getOrthogonalMovement(square, blockerBoard);
}

uint64_t MovePrecomputation::getDiagMovementBoard(const int& square, const uint64_t& blockerBoard) {
	return magics->getDiagonalMovement(square, blockerBoard);
}

uint64_t MovePrecomputation::getKnightBoard(const int& square) {
	return knightMoves[square];
}

uint64_t MovePrecomputation::getKingMoves(const int& square) {
	return kingMoves[square];
}

uint64_t MovePrecomputation::getPawnAttackMoves(const int& square, const int& color) {
	return color == Piece::white ? whitePawnAttackMoves[square] : blackPawnAttackMoves[square];
}

uint64_t MovePrecomputation::getPassedPawnMask(const int& square, const int& color) {
	return color == Piece::white ? whitePassedPawnMasks[square] : blackPassedPawnMasks[square];
}

uint64_t MovePrecomputation::getPawnIslandMask(const int& column) {
	return pawnIslandMasks[column];
}

uint64_t MovePrecomputation::getShieldMask(const int& square, const int& color) {
	return color == Piece::white ? whitePawnShieldMask[square] : blackPawnShieldMask[square];
}

uint64_t MovePrecomputation::getDirectionMask(const int& square1, const int& square2) {	
	if (square1 / 8 == square2 / 8) {
		if (square2 > square1) {
			return directionMasks[square1 * 8 + 1];
		} else {
			return directionMasks[square1 * 8 + 3];
		}
	} else if (square1 % 8 == square2 % 8) {
		if (square2 > square1) {
			return directionMasks[square1 * 8 + 2];
		} else {
			return directionMasks[square1 * 8];
		}
	}
	int dir = differenceDivisibles[64 + square2 - square1];
	

	return directionMasks[square1 * 8 + dir];
}

unsigned char MovePrecomputation::getDistance(const int& square1, const int& square2) {
	return distances[square1][square2];
}



MovePrecomputation::dirDist::dirDist() {
}

MovePrecomputation::dirDist::dirDist(int top, int left, int right, int bottom) : top(top), left(left), right(right), bottom(bottom) {
	direction[0] = top;
	direction[1] = right;
	direction[2] = bottom;
	direction[3] = left;
	direction[4] = std::min(left, top);
	direction[5] = std::min(right, top);
	direction[6] = std::min(right, bottom);
	direction[7] = std::min(left, bottom);

	knightSquares[0] = left > 1 && top > 2;
	knightSquares[1] = right > 1 && top > 2;
	knightSquares[2] = right > 2 && top > 1;
	knightSquares[3] = right > 2 && bottom > 1;
	knightSquares[4] = right > 1 && bottom > 2;
	knightSquares[5] = left > 1 && bottom > 2;
	knightSquares[6] = left > 2 && bottom > 1;
	knightSquares[7] = left > 2 && top > 1;

	this->minVertical = std::min(top, bottom);
	this->minHorizontal = std::min(left, right);
}