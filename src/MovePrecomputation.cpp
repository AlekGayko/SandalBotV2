#include "MovePrecomputation.h"

#include "BitBoardUtility.h"

#include <cmath>
#include <iostream>
#include <limits>

using namespace std;

namespace SandalBot {

	// Default constructor populates all member arrays and data
	MovePrecomputation::MovePrecomputation() {
		// Initialise distances to edge of board
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
		initMasks(); // Compute necessary masks
		precomputeMoves(); // Compute magic bitboard (hashtable for orthogonal and diagonal moves)
	}

	// Initialises all masks
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

	// Initialises 45 degree diagonal masks
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

	// Initialises -45 degree diagonal masks
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

	// Initialises direction masks, an array of bitboards of lines from each square to each
	// direction. Also initialises difference divisibles which is used to index, the directionMasks
	// by storing the direction between two squares
	void MovePrecomputation::initDirectionMasks() {
		for (int square = 0; square < 64; square++) {
			// Calculate direction masks
			for (int dirIndex = startOrthogonal; dirIndex < endDiagonal; dirIndex++) {
				directionMasks[square * 8 + dirIndex] = 0ULL;
				int distance = directionDistances[square].direction[dirIndex];
				// Add bits to bitboard along direction from given square
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

	// Initialises passed pawn masks
	void MovePrecomputation::initPassedPawnMasks() {
		for (int square = 0; square < 64; square++) {
			// Create masks which cover the rows in front of a pawn
			uint64_t whiteFrontMask = numeric_limits<uint64_t>::max() >> min(((7 - square / 8) + 1), 7) * 8;
			uint64_t blackFrontMask = numeric_limits<uint64_t>::max() << min((square / 8 + 1), 7) * 8;

			// Create column mask for current column and adjacent ones
			uint64_t columnMask = columnMasks[square % 8];
			columnMask |= columnMasks[min((square % 8) + 1, 7)];
			columnMask |= columnMasks[max((square % 8) - 1, 0)];

			// And operation on both masks creates a mask covering all squares in front of pawn
			// that are on the columns affecting the pawn
			whitePassedPawnMasks[square] = whiteFrontMask & columnMask;
			blackPassedPawnMasks[square] = blackFrontMask & columnMask;
		}
	}

	// Initialises pawn island masks
	void MovePrecomputation::initIslandMasks() {
		// For each column
		for (int col = 0; col < 8; col++) {
			// Create mask of adjacent columns (pawns on these columns can
			// prevent pawn islands)
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

	// Initialises pawn shield mask for in front of black and white king
	void MovePrecomputation::initShieldMasks() {
		for (int square = 0; square < 64; square++) {
			// Masks which covers rows in front of king
			uint64_t whiteMask = (numeric_limits<uint64_t>::max() << 6 * 8) >> min(((7 - square / 8) + 1), 7) * 8;
			uint64_t blackMask = (numeric_limits<uint64_t>::max() >> 6 * 8) << min((square / 8 + 1), 7) * 8;

			// Mask for three columns about king (extended one colum further if
			// one edge of board e.g. a1 king includes c file but b1 king does not
			// include d file) 
			uint64_t colMask = 0ULL;

			int col = square % 8;
			colMask |= columnMasks[col];

			if (col == 0) {
				colMask |= columnMasks[col + 2];
			} else {
				colMask |= columnMasks[col - 1];
			}

			if (col == 7) {
				colMask |= columnMasks[col - 2];
			} else {
				colMask |= columnMasks[1 + col];
			}

			whiteMask &= colMask;
			blackMask &= colMask;

			// Masks for pawn shields in front of king
			whitePawnShieldMask[square] = whiteMask;
			blackPawnShieldMask[square] = blackMask;
		}
	}

	// Initialises Chebyshev distances between two squares
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

	// Initialises bitboards for 'attack' zone near king - the area susceptible to attacks
	void MovePrecomputation::initKingAttackSquares() {
		// Set attack zones to immediate squares and use pawn shield mask
		// if on side's back rank
		for (int square = 0; square < 64; square++) {
			uint64_t immediateSquares = kingMoves[square];
			uint64_t whiteMask = immediateSquares;
			uint64_t blackMask = immediateSquares;

			if (square / 8 > 1)
				whiteMask |= whitePawnShieldMask[square - 8];
			if (square / 8 < 6)
				blackMask |= blackPawnShieldMask[square + 8];

			whiteKingAttackZone[square] = whiteMask;
			blackKingAttackZone[square] = blackMask;
		}
		// For each square on board, add square of bits to attack zone
		// especially greater zones the closer to the center of the board
		for (int square = 0; square < 64; square++) {
			int row = square / 8;
			int column = square % 8;

			uint64_t rows = rowMasks[row];
			uint64_t columns = columnMasks[column];

			// Closer to center row, the more rows in attack zone
			if (row > 0)
				rows |= rowMasks[row - 1];
			if (row > 1)
				rows |= rowMasks[row - 2];
			if (row < 7)
				rows |= rowMasks[row + 1];
			if (row < 6)
				rows |= rowMasks[row + 2];
			// Closer to center column, the more columns in attack zone
			if (column > 0)
				columns |= columnMasks[column - 1];
			if (column > 1)
				columns |= columnMasks[column - 2];
			if (column < 7)
				columns |= columnMasks[column + 1];
			if (column < 6)
				columns |= columnMasks[column + 2];

			uint64_t zone = rows & columns;
			zone &= ~(1ULL << square);

			whiteKingAttackZone[square] |= zone;
			blackKingAttackZone[square] |= zone;
		}
	}

	// Computes movement bitboards for each piece type
	void MovePrecomputation::precomputeMoves() {
		precomputeOrthogonalMoves();
		precomputeDiagonalMoves();
		precomputeKnightMoves();
		precomputeKingMoves();
		precomputePawnMoves();
	}

	// Computes all orthogonal moves
	void MovePrecomputation::precomputeOrthogonalMoves() {
		for (int square = 0; square < 64; square++) {
			// Generate all potential blocker squares - excluding esge squares
			// since edge piece cannot block anything behind them
			vector<uint64_t> blockerConfigs = precomputeOrthogonalMove(square);
			// Need blockerConfigs.size() movement boards to accomodate each permutation
			// of blockers
			vector<uint64_t> movementBoards;
			movementBoards.reserve(blockerConfigs.size());

			// For each blockerboard, generate a movement bitboard for that permutation
			// of blockers
			for (uint64_t& blockerBoard : blockerConfigs) {
				uint64_t moves = createOrthogonalMovement(square, blockerBoard);
				movementBoards.push_back(moves);
			}

			// Add all orthogonal movement bitboards for each blocker permutation
			magics.addOrthogonalMoves(square, blockerConfigs, movementBoards);
		}
	}

	// Computes all orthogonal moves
	void MovePrecomputation::precomputeDiagonalMoves() {
		for (int square = 0; square < 64; square++) {
			// Generate all potential blocker squares - excluding esge squares
			// since edge piece cannot block anything behind them
			vector<uint64_t> blockerConfigs = precomputeDiagonalMove(square);
			// Need blockerConfigs.size() movement boards to accomodate each permutation
			// of blockers
			vector<uint64_t> movementBoards;
			movementBoards.reserve(blockerConfigs.size());

			// For each blockerboard, generate a movement bitboard for that permutation
			// of blockers
			for (uint64_t& blockerBoard : blockerConfigs) {
				uint64_t moves = createDiagonalMovement(square, blockerBoard);
				movementBoards.push_back(moves);
			}

			// Add all orthogonal movement bitboards for each blocker permutation
			magics.addDiagonalMoves(square, blockerConfigs, movementBoards);
		}
	}

	// Precompute all knight movement bitboards
	void MovePrecomputation::precomputeKnightMoves() {
		for (int square = 0; square < 64; square++) {
			// For each square add bits for every square knight can land
			knightMoves[square] = 0ULL;
			for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
				if (directionDistances[square].knightSquares[dirIndex]) {
					knightMoves[square] |= 1ULL << (square + knightDirections[dirIndex]);
				}
			}
		}
	}

	// Precompute all king movement bitboards
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
		initKingAttackSquares(); // Initialise attack zones
	}

	// Initialise all pawn attack moves
	void MovePrecomputation::precomputePawnMoves() {
		for (int square = 0; square < 64; square++) {
			// For each square, generate movement bitboard for white and black
			// pawns' forward diagonal directions
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

	// Computes all orthogonally available moves from a square (not including edges)
	std::vector<uint64_t> MovePrecomputation::precomputeOrthogonalMove(int square) {
		vector<int> moveSquares;
		// Calculate orthogonal moves
		for (int i = 0; i < 64; i++) {
			int row = i / 8;
			int col = i % 8;
			if (i == square) { // Cannot move to own square
				continue;
			} 
			// Do not mark edges if square is not on that edge
			else if ((square / 8 != 0 && row == 0) || (square / 8 != 7 && row == 7)) {
				continue;
			} else if ((square % 8 != 0 && col == 0) || (square % 8 != 7 && col == 7)) {
				continue;
			} 
			// If on same column or row mark as a square
			else if (square / 8 == i / 8) {
				moveSquares.push_back(i);
			} else if (square % 8 == i % 8) {
				moveSquares.push_back(i);
			}

		}

		return makeConfigs(moveSquares); // Return permutation of blockers
	}

	// Computes all orthogonally available moves from a square (not including edges)
	std::vector<uint64_t> MovePrecomputation::precomputeDiagonalMove(int square) {
		vector<int> moveSquares;
		// Calculate diagonal moves
		// For each diagonal direction
		for (int dirIndex = startDiagonal; dirIndex < endDiagonal; dirIndex++) {
			int distance = directionDistances[square].direction[dirIndex];
			// Iterate in direction from given square
			for (int it = 1; it < distance; it++) {
				int newSquare = square + slideDirections[dirIndex] * it;
				int row = newSquare / 8;
				int col = newSquare % 8;
				// If edge square skip
				if ((square / 8 != 0 && row == 0) || (square / 8 != 7 && row == 7)) {
					break;
				} else if ((square % 8 != 0 && col == 0) || (square % 8 != 7 && col == 7)) {
					break;
				}
				moveSquares.push_back(newSquare);
			}
		}

		return makeConfigs(moveSquares); // Return permutations of blockers
	}

	// Returns a vector of movement boards for each set of blockers
	std::vector<uint64_t> MovePrecomputation::makeConfigs(std::vector<int>& moveSquares) {
		int n = moveSquares.size();
		int numConfigs = 1 << n; // 2^n (number of permutations of blockers)
		std::vector<uint64_t> blockerConfigs(numConfigs, 0ULL);

		// Pattern = permutation number up to 2^n
		for (int pattern = 0; pattern < numConfigs; pattern++) {
			// Pattern is therefore at most n bit, bitIndex indexes these bit in pattern
			for (int bitIndex = 0; bitIndex < n; bitIndex++) {
				// bit is the current bit indexed by bitIndex on pattern
				uint64_t bit = (pattern & (1ULL << bitIndex)) >> bitIndex;
				// Shifts the bit (0 or 1) to the position of corresponding move square,
				// and adds it to blocker board. Essentially mapping each bit from pattern
				// onto a square from the set of blockers to create unique key
				blockerConfigs[pattern] |= bit << moveSquares[bitIndex];
			}

		}

		return blockerConfigs;
	}

	// Initialises orthogonal mask for a square in blockerOrthogonalMasks
	uint64_t MovePrecomputation::createOrthogonalMask(int square) {
		uint64_t mask = 0ULL;
		mask |= blockerRowMask << (square / 8) * 8; // Add row mask

		mask |= blockerColumnMask << (square % 8); // Add column mask
		mask &= ~(1ULL << square); // Remove current square

		blockerOrthogonalMasks[square] = mask;

		return mask;
	}

	// Initialises diagonal mask for a square in blockerDiagonalMasks
	uint64_t MovePrecomputation::createDiagonalMask(int square) {
		uint64_t mask = 0ULL;

		// For each diagonal direction
		for (int dirIndex = startDiagonal; dirIndex < endDiagonal; dirIndex++) {
			int distance = directionDistances[square].direction[dirIndex];
			// Increase direction by scalar distance
			for (int it = 1; it < distance; it++) {
				int newSquare = square + slideDirections[dirIndex] * it;
				int row = newSquare / 8;
				int col = newSquare % 8;
				// If square on edge don't add to blocker mask
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

	// Create a orthogonal movement board for a square and set of blockers
	uint64_t MovePrecomputation::createOrthogonalMovement(int square, uint64_t blockerBoard) {
		return createMovement(square, blockerBoard, startOrthogonal, endOrthogonal);
	}
	// Create a diagonal movement board for a square and set of blockers
	uint64_t MovePrecomputation::createDiagonalMovement(int square, uint64_t blockerBoard) {
		return createMovement(square, blockerBoard, startDiagonal, endDiagonal);
	}
	// Returns movement bitboard from a square, set of blockers, and indexes for direction
	uint64_t MovePrecomputation::createMovement(int square, uint64_t blockerBoard, int start, int end) {
		uint64_t movementBoard = 0ULL;

		// For each specified direction
		for (int dirIndex = start; dirIndex < end; dirIndex++) {
			int distance = directionDistances[square].direction[dirIndex];
			// Add movement square to bitboard
			for (int it = 1; it < distance; it++) {
				int newSquare = square + slideDirections[dirIndex] * it;

				movementBoard |= 1ULL << newSquare;

				// If blocker reached, cannot add more squares behind the blocker
				if (BitBoardUtility::getBit(blockerBoard, newSquare)) {
					break;
				}
			}
		}

		return movementBoard;
	}

	// Returns directional mask from square1 towards the direction of square2 until the end of the board
	uint64_t MovePrecomputation::getDirectionMask(const int square1, const int square2) {
		// If both squares on same row, horizontal direction
		if (square1 / 8 == square2 / 8) {
			if (square2 > square1) {
				return directionMasks[square1 * 8 + 1];
			} else {
				return directionMasks[square1 * 8 + 3];
			}
		} 
		// If squares on same column, vertical direction
		else if (square1 % 8 == square2 % 8) {
			if (square2 > square1) {
				return directionMasks[square1 * 8 + 2];
			} else {
				return directionMasks[square1 * 8];
			}
		}
		// Else, diagonal direction (use difference divisibles to get direction)
		int dir = differenceDivisibles[64 + square2 - square1];


		return directionMasks[square1 * 8 + dir]; // Return diagonal direction mask
	}

	// dirDist constructor, initialising distance to edges of board
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

}