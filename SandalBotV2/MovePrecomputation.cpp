#include "MovePrecomputation.h"

#include "BitBoardUtility.h"
#include "Precomputedmagics.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <unordered_map>
#include <vector>
#include <set>
#include <cmath>

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
		//cout << "row: " << row << ", col: " << col << endl;
		if (row >= 0 && row < 8) {
			mask |= 1ULL << (row * 8 + col);
		}
		col++;
		row = constant + col;
	}

	backwardDiagonalMasks[constant + 7] = mask;
}

void MovePrecomputation::precomputeMoves() {
	precomputeOrthogonalMoves();
	precomputeDiagonalMoves();
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
	cout << "orth size: " << size << endl;
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
	cout << "diag size: " << size << endl;
}

uint64_t get_current_time_in_milliseconds() {
	// Get the current time point from the system clock
	auto now = std::chrono::system_clock::now();

	// Convert the time point to a duration since the epoch
	auto duration = now.time_since_epoch();

	// Convert the duration to milliseconds
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	return milliseconds;
}

uint64_t random_uint64() {
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64_t> dist(0ULL, std::numeric_limits<uint64_t>::max());
	uint64_t u1, u2, u3, u4;
	u1 = (uint64_t)(dist(gen)) & 0xFFFF; u2 = (uint64_t)(dist(gen)) & 0xFFFF;
	u3 = (uint64_t)(dist(gen)) & 0xFFFF; u4 = (uint64_t)(dist(gen)) & 0xFFFF;
	return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}
/*
void MovePrecomputation::generateMagicNumbers(vector<uint64_t>& blockerConfigs, bool isOrth) {
	uint64_t magicNumber = 0ULL;
	int size = isOrth ? orthMagicNumbers.size() : diagMagicNumbers.size();
	const int minRightShift = 63 - (log2(blockerConfigs.size()) + 3);
	const int maxRightShift = 63 - (log2(blockerConfigs.size()));
	cout << "size: " << blockerConfigs.size() << endl;
	cout << "maxshift: " << maxRightShift << ", minshift: " << minRightShift << endl;

	auto nextPrintTime = get_current_time_in_milliseconds() + 500ULL;
	uint64_t acceptableSize = blockerConfigs.size() * 2;

	uint64_t bestMagic = 0ULL;
	int bestRightShift = -1;
	uint64_t maxIndex = std::numeric_limits<uint64_t>::max();

	uint64_t indexesSize = 1ULL << (68 - maxRightShift);
	bool* indexes = new bool[indexesSize];
	while (maxIndex > acceptableSize) {
		// Random magic number
		std::random_device rd;
		std::mt19937_64 gen(rd());
		std::uniform_int_distribution<uint64_t> dist(0ULL, std::numeric_limits<uint64_t>::max());
		magicNumber = dist(gen);

		for (int rightShift = minRightShift; rightShift <= maxRightShift; rightShift++) {
			for (int i = 0; i < indexesSize; i++)
				indexes[i] = false;
			uint64_t bestIndex = numeric_limits<uint64_t>::min();
			bool duplicates = false;
			for (uint64_t& blocker : blockerConfigs) {
				uint64_t index = blocker * magicNumber;
				index >>= rightShift;
				if (indexes[index] == true) {
					duplicates = true;
					break;
				}
				indexes[index] = true;
				bestIndex = max(bestIndex, index);
			}
			if (duplicates)
				continue;
			if (bestIndex < maxIndex) {
				bestMagic = magicNumber;
				bestRightShift = rightShift;
				maxIndex = bestIndex;
			}
		}
		if (get_current_time_in_milliseconds() > nextPrintTime) {
			cout << "maxIndex: " << maxIndex << ", square: " << size << endl;
			nextPrintTime = get_current_time_in_milliseconds() + 500;
		}
	}
	delete[] indexes;
	if (bestRightShift == -1) {
		cout << "FAILUUUUUUUUUUUUUURE!!!!" << endl;
	}
	cout << "Magic Number Found!: " << maxIndex << ", Square: " << size << endl;
	if (isOrth) {
		orthMagicNumbers.push_back(bestMagic);
		orthRightShifts.push_back(bestRightShift);
		maxOrthIndex.push_back(maxIndex);
	} else {
		diagMagicNumbers.push_back(bestMagic);
		diagRightShifts.push_back(bestRightShift);
		maxDiagIndex.push_back(maxIndex);
	}
}
*/

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