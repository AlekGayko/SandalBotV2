#include "Bitboards.h"

#include <cmath>
#include <iostream>
#include <limits>

using namespace std;

namespace SandalBot {

	dirDist directionDistances[SQUARES_NB];

	uint8_t distances[SQUARES_NB][SQUARES_NB]; // Stores distance between any two coordinates
	// King and knight movement bitboards
	// Pawn shield bitboard masks for king
	Bitboard pawnShieldMask[COLOR_NB][SQUARES_NB];
	// Bitboards for attack zone around king
	Bitboard kingAttackZone[COLOR_NB][SQUARES_NB];
	// Bitboard attack zone for either white or black king
	Bitboard kingUnbiasAttackZone[SQUARES_NB];
	// Movement bitboards for attack moves for white and black pawns
	Bitboard pawnAttackMoves[COLOR_NB][SQUARES_NB];
	// Bitboard masks for white and black pawns for passed pawns.
	// Masks pawn's column and adjacent columns in front of pawn
	Bitboard passedPawnMasks[COLOR_NB][SQUARES_NB];
	// Bitboard masks for each column for pawn islands. (adjacent column masks)
	Bitboard pawnIslandMasks[ROW_NB];
	// Bitboard masks for all orthogonal and diagonal directions
	Bitboard blockerOrthogonalMasks[SQUARES_NB];
	Bitboard blockerDiagonalMasks[SQUARES_NB];
	// Masks for all rows and columns
	Bitboard rowMasks[ROW_NB];
	Bitboard columnMasks[COL_NB];
	Bitboard forwardDiagonalMasks[15]; // 45 degree diagonals
	Bitboard backwardDiagonalMasks[15]; // -45 degree diagonals

	Bitboard movementBoards[PIECE_TYPE_NB][SQUARES_NB];

	Bitboard linesBB[SQUARES_NB][SQUARES_NB];
	Bitboard linesBetweenBB[SQUARES_NB][SQUARES_NB];

	// Returns a bitmask of bounding box between two squares
	Bitboard boxMask(Square sq1, Square sq2) {
		Bitboard horizontalMask = 0ULL;
		Bitboard verticalMask = 0ULL;

		Row row1 = toRow(sq1);
		Column col1 = toCol(sq1);

		Row row2 = toRow(sq2);
		Column col2 = toCol(sq2);

		Row rStart = row1 <= row2 ? row1 : row2;
		Row rEnd = row1 <= row2 ? row2 : row1;

		for (Row r = rStart; r <= rEnd; ++r) {
			horizontalMask |= getRowMask(Square(r * ROW_NB));
		}

		Column cStart = col1 <= col2 ? col1 : col2;
		Column cEnd = col1 <= col2 ? col2 : col1;

		for (Column c = cStart; c <= cEnd; ++c) {
			verticalMask |= getColMask(Square(c));
		}

		return horizontalMask & verticalMask;
	}

	// dirDist constructor, initialising distance to edges of board
	dirDist::dirDist(int top, int left, int right, int bottom) {
		slideDistances[NORTH_IDX] = top;
		slideDistances[EAST_IDX] = right;
		slideDistances[SOUTH_IDX] = bottom;
		slideDistances[WEST_IDX] = left;
		slideDistances[NORTH_WEST_IDX] = std::min(left, top);
		slideDistances[NORTH_EAST_IDX] = std::min(right, top);
		slideDistances[SOUTH_EAST_IDX] = std::min(right, bottom);
		slideDistances[SOUTH_WEST_IDX] = std::min(left, bottom);

		knightSquares[0] = left > 1 && top > 2;
		knightSquares[1] = right > 1 && top > 2;
		knightSquares[2] = right > 2 && top > 1;
		knightSquares[3] = right > 2 && bottom > 1;
		knightSquares[4] = right > 1 && bottom > 2;
		knightSquares[5] = left > 1 && bottom > 2;
		knightSquares[6] = left > 2 && bottom > 1;
		knightSquares[7] = left > 2 && top > 1;
	}

	static void initDirectionDistances() {
		// Initialise distances to edge of board
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			int top, left, right, bottom;
			Row row = toRow(square);
			Column col = toCol(square);
			top = row + 1;
			bottom = 8 - row;
			left = col + 1;
			right = 8 - col;
			directionDistances[square] = dirDist(top, left, right, bottom);
		}
	}
	
	static void initLinesBB() {
		for (Square sq1 = START_SQUARE; sq1 < SQUARES_NB; ++sq1) {
			for (Square sq2 = START_SQUARE; sq2 < SQUARES_NB; ++sq2) {
				linesBB[sq1][sq2] = 0ULL;
				linesBetweenBB[sq1][sq2] = 0ULL;
				if (sq1 == sq2) {
					continue;
				}

				Row row1 = toRow(sq1);
				Column col1 = toCol(sq1);

				Row row2 = toRow(sq2);
				Column col2 = toCol(sq2);

				if (row1 == row2) {
					linesBB[sq1][sq2] = getRowMask(sq1);
				} else if (col1 == col2) {
					linesBB[sq1][sq2] = getColMask(sq1);
				} else if ((row1 - row2) == (col1 - col2)) {
					linesBB[sq1][sq2] = getBackwardMask(sq1);
				} else if ((row1 - row2) == -(col1 - col2)) {
					linesBB[sq1][sq2] = getForwardMask(sq1);
				} else {
					continue;
				}

				linesBetweenBB[sq1][sq2] = linesBB[sq1][sq2] & boxMask(sq1, sq2);
			}    
		}
	}

	// Initialises 45 degree diagonal masks
	static void initForwardMask(int constant) {
		Bitboard mask = 0ULL;
		Column col = COL_START;
		Row row = Row(constant - col);

		while (col < COL_NB) {
			if (row >= ROW_START && row < ROW_NB) {
				mask |= 1ULL << (row * 8 + col);
			}
			++col;
			row = Row(constant - col);
		}

		forwardDiagonalMasks[constant] = mask;
	}

	// Initialises -45 degree diagonal masks
	static void initBackwardMask(int constant) {
		constant = constant - 7;
		Bitboard mask = 0ULL;
		Column col = COL_START;
		Row row = Row(constant + col);

		while (col < COL_NB) {
			if (row >= ROW_START && row < ROW_NB) {
				mask |= 1ULL << (row * 8 + col);
			}
			++col;
			row = Row(constant + col);
		}

		backwardDiagonalMasks[constant + 7] = mask;
	}

	// Initialises passed pawn masks
	static void initPassedPawnMasks() {
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			// Create masks which cover the rows in front of a pawn
			Bitboard whiteFrontMask = numeric_limits<Bitboard>::max() >> min(((7 - toRow(square)) + 1), 7) * 8;
			Bitboard blackFrontMask = numeric_limits<Bitboard>::max() << min((toRow(square) + 1), 7) * 8;

			// Create column mask for current column and adjacent ones
			Bitboard columnMask = columnMasks[toCol(square)];
			columnMask |= columnMasks[min(toCol(square) + 1, 7)];
			columnMask |= columnMasks[max(toCol(square) - 1, 0)];

			// And operation on both masks creates a mask covering all squares in front of pawn
			// that are on the columns affecting the pawn
			passedPawnMasks[WHITE][square] = whiteFrontMask & columnMask;
			passedPawnMasks[BLACK][square] = blackFrontMask & columnMask;
		}
	}

	// Initialises pawn island masks
	static void initIslandMasks() {
		// For each column
		for (Column col = COL_START; col < COL_NB; ++col) {
			// Create mask of adjacent columns (pawns on these columns can
			// prevent pawn islands)
			Bitboard mask = 0ULL;
			if (directionDistances[col].slideDistances[EAST_IDX] > 1) {
				mask |= columnMasks[col + 1];
			}
			if (directionDistances[col].slideDistances[WEST_IDX] > 1) {
				mask |= columnMasks[col - 1];
			}

			pawnIslandMasks[col] = mask;
		}
	}

	// Initialises pawn shield mask for in front of black and white king
	// Mask for three columns about king (extended one colum further if
	// one edge of board e.g. a1 king includes c file but b1 king does not
	// include d file) 
	static void initShieldMasks() {
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			Square wSq1 = square + NORTH_WEST;
			Square wSq2 = square + NORTH_EAST + NORTH + NORTH;

			Square bSq1 = square + SOUTH_WEST;
			Square bSq2 = square + SOUTH_EAST + SOUTH + SOUTH;

			Column col = toCol(square);

			if (col == COL_A) {
				wSq1 = wSq1 + EAST;
				wSq2 = wSq2 + EAST;

				bSq1 = bSq1 + EAST;
				bSq2 = bSq2 + EAST;
			} else if (col == COL_H) {
				wSq1 = wSq1 + WEST;
				wSq2 = wSq2 + WEST;

				bSq1 = bSq1 + WEST;
				bSq2 = bSq2 + WEST;
			}

			// Masks which covers rows in front of king
			Bitboard whiteMask = boxMask(wSq1, wSq2);
			Bitboard blackMask = boxMask(bSq1, bSq2);

			// Masks for pawn shields in front of king
			pawnShieldMask[WHITE][square] = whiteMask;
			pawnShieldMask[BLACK][square] = blackMask;
		}
	}

	// Initialises Chebyshev distances between two squares
	static void initDistances() {
		for (Square square1 = START_SQUARE; square1 < SQUARES_NB; ++square1) {
			for (Square square2 = START_SQUARE; square2 < SQUARES_NB; ++square2) {
				Column file1, file2;
				Row rank1, rank2;
				int rankDistance, fileDistance;
				file1 = toCol(square1);
				file2 = toCol(square2);
				rank1 = toRow(square1);
				rank2 = toRow(square2);
				rankDistance = abs(rank2 - rank1);
				fileDistance = abs(file2 - file1);
				distances[square1][square2] = max(rankDistance, fileDistance);
			}
		}
	}

	// Initialises bitboards for 'attack' zone near king - the area susceptible to attacks
	static void initKingAttackSquares() {
		// Set attack zones to immediate squares and use pawn shield mask
		// if on side's back rank
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			Bitboard immediateSquares = movementBoards[KING][square];
			Bitboard whiteMask = immediateSquares;
			Bitboard blackMask = immediateSquares;

			if (toRow(square) > 1)
				whiteMask |= pawnShieldMask[WHITE][square + pawnPush(WHITE)];
			if (toRow(square) < 6)
				blackMask |= pawnShieldMask[BLACK][square + pawnPush(BLACK)];

			kingAttackZone[WHITE][square] = whiteMask;
			kingAttackZone[BLACK][square] = blackMask;
		}
		// For each square on board, add square of bits to attack zone
		// especially greater zones the closer to the center of the board
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			Row row = toRow(square);
			Column column = toCol(square);

			Bitboard rows = rowMasks[row];
			Bitboard columns = columnMasks[column];

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

			Bitboard zone = rows & columns;
			zone &= ~(1ULL << square);

			kingAttackZone[WHITE][square] |= zone;
			kingAttackZone[BLACK][square] |= zone;
		}
	}

	// Precompute all knight movement bitboards
	static void precomputeKnightMoves() {
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			// For each square add bits for every square knight can land
			movementBoards[KNIGHT][square] = 0ULL;
			for (DistIndex dirIndex = DISTINDEX_START; dirIndex < DISTINDEX_NB; ++dirIndex) {
				if (directionDistances[square].knightSquares[dirIndex]) {
					movementBoards[KNIGHT][square] |= 1ULL << (square + knightDirections[dirIndex]);
				}
			}
		}
	}

	// Precompute all king movement bitboards
	static void precomputeKingMoves() {
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			movementBoards[KING][square] = 0ULL;
			// Calculate diagonal moves
			for (DistIndex dirIndex = DISTINDEX_START; dirIndex < DISTINDEX_NB; ++dirIndex) {
				int distance = directionDistances[square].slideDistances[dirIndex];
				if (distance <= 1)
					continue;

				movementBoards[KING][square] |= 1ULL << (square + slideDirections[dirIndex]);
			}
		}
		initKingAttackSquares(); // Initialise attack zones
	}

	// Initialise all pawn attack moves
	static void precomputePawnMoves() {
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			// For each square, generate movement bitboard for white and black
			// pawns' forward diagonal directions
			pawnAttackMoves[WHITE][square] = 0ULL;
			pawnAttackMoves[BLACK][square] = 0ULL;
			// White pawns
			if (directionDistances[square].slideDistances[NORTH_WEST_IDX] > 1) {
				pawnAttackMoves[WHITE][square] |= 1ULL << (square + NORTH_WEST);
			}
			if (directionDistances[square].slideDistances[NORTH_EAST_IDX] > 1) {
				pawnAttackMoves[WHITE][square] |= 1ULL << (square + NORTH_EAST);
			}
			// Black pawns
			if (directionDistances[square].slideDistances[SOUTH_EAST_IDX] > 1) {
				pawnAttackMoves[BLACK][square] |= 1ULL << (square + SOUTH_EAST);
			}
			if (directionDistances[square].slideDistances[SOUTH_WEST_IDX] > 1) {
				pawnAttackMoves[BLACK][square] |= 1ULL << (square + SOUTH_WEST);
			}
		}
	}

	// Returns a vector of movement boards for each set of blockers
	static std::vector<Bitboard> makeConfigs(std::vector<Square>& moveSquares) {
		int n = moveSquares.size();
		int numConfigs = 1 << n; // 2^n (number of permutations of blockers)
		std::vector<Bitboard> blockerConfigs(numConfigs, 0ULL);

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

	// Computes all orthogonally available moves from a square (not including edges)
	static std::vector<Bitboard> precomputeOrthogonalMove(Square square) {
		vector<Square> moveSquares;
		Row row = toRow(square);
		Column col = toCol(square);

		// Calculate orthogonal moves
		for (Square i = START_SQUARE; i < SQUARES_NB; ++i) {
			Row iRow = toRow(i);
			Column iCol = toCol(i);
			if (i == square) { // Cannot move to own square
				continue;
			}
			// Do not mark edges if square is not on that edge
			else if ((row != 0 && iRow == 0) || (row != 7 && iRow == 7)) {
				continue;
			} else if ((col != 0 && iCol == 0) || (col != 7 && iCol == 7)) {
				continue;
			}
			// If on same column or row mark as a square
			else if (row == iRow) {
				moveSquares.push_back(i);
			} else if (col == iCol) {
				moveSquares.push_back(i);
			}

		}

		return makeConfigs(moveSquares); // Return permutation of blockers
	}

	// Computes all orthogonally available moves from a square (not including edges)
	static std::vector<Bitboard> precomputeDiagonalMove(Square square) {
		vector<Square> moveSquares;
		Row row = toRow(square);
		Column col = toCol(square);

		// Calculate diagonal moves
		// For each diagonal direction
		for (DistIndex dirIndex = START_DIAG; dirIndex < END_DIAG; ++dirIndex) {
			int distance = directionDistances[square].slideDistances[dirIndex];
			// Iterate in direction from given square
			for (int it = 1; it < distance; it++) {
				Square newSquare = Square(square + slideDirections[dirIndex] * it);
				Row newRow = toRow(newSquare);
				Column newCol = toCol(newSquare);
				// If edge square skip
				if ((row != 0 && newRow == 0) || (row != 7 && newRow == 7)) {
					break;
				} else if ((col != 0 && newCol == 0) || (col != 7 && newCol == 7)) {
					break;
				}
				moveSquares.push_back(newSquare);
			}
		}

		return makeConfigs(moveSquares); // Return permutations of blockers
	}

	// Initialises orthogonal mask for a square in blockerOrthogonalMasks
	static Bitboard createOrthogonalMask(Square square) {
		Bitboard mask = 0ULL;
		mask |= blockerRowMask << (toRow(square)) * 8; // Add row mask

		mask |= blockerColumnMask << toCol(square); // Add column mask
		mask &= ~(1ULL << square); // Remove current square

		blockerOrthogonalMasks[square] = mask;

		return mask;
	}

	// Initialises diagonal mask for a square in blockerDiagonalMasks
	static Bitboard createDiagonalMask(Square square) {
		Bitboard mask = 0ULL;

		// For each diagonal direction
		for (DistIndex dirIndex = START_DIAG; dirIndex < END_DIAG; ++dirIndex) {
			int distance = directionDistances[square].slideDistances[dirIndex];
			// Increase direction by scalar distance
			for (int it = 1; it < distance; it++) {
				Square newSquare = Square(square + slideDirections[dirIndex] * it);
				Row row = toRow(newSquare);
				Column col = toCol(newSquare);
				// If square on edge don't add to blocker mask
				if ((toRow(square) != 0 && row == 0) || (toRow(square) != 7 && row == 7)) {
					break;
				} else if ((toCol(square) != 0 && col == 0) || (toCol(square) != 7 && col == 7)) {
					break;
				}
				mask |= 1ULL << newSquare;
			}
		}
		blockerDiagonalMasks[square] = mask;

		return mask;
	}

	// Returns movement bitboard from a square, set of blockers, and indexes for direction
	static Bitboard createMovement(Square square, Bitboard blockerBoard, DistIndex start, DistIndex end) {
		Bitboard movementBoard = 0ULL;

		// For each specified direction
		for (DistIndex dirIndex = start; dirIndex < end; ++dirIndex) {
			int distance = directionDistances[square].slideDistances[dirIndex];
			// Add movement square to bitboard
			for (int it = 1; it < distance; it++) {
				Square newSquare = Square(square + slideDirections[dirIndex] * it);

				movementBoard |= 1ULL << newSquare;

				// If blocker reached, cannot add more squares behind the blocker
				if (getBit(blockerBoard, newSquare)) {
					break;
				}
			}
		}

		return movementBoard;

	}

	// Create a orthogonal movement board for a square and set of blockers
	static Bitboard createOrthogonalMovement(Square square, Bitboard blockerBoard) {
		return createMovement(square, blockerBoard, START_ORTH, END_ORTH);
	}
	// Create a diagonal movement board for a square and set of blockers
	static Bitboard createDiagonalMovement(Square square, Bitboard blockerBoard) {
		return createMovement(square, blockerBoard, START_DIAG, END_DIAG);
	}

	// Computes all orthogonal moves
	static void precomputeOrthogonalMoves() {
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			// Generate all potential blocker squares - excluding esge squares
			// since edge piece cannot block anything behind them
			vector<Bitboard> blockerConfigs = precomputeOrthogonalMove(square);
			// Need blockerConfigs.size() movement boards to accomodate each permutation
			// of blockers
			vector<Bitboard> movementBoards;
			movementBoards.reserve(blockerConfigs.size());

			// For each blockerboard, generate a movement bitboard for that permutation
			// of blockers
			for (Bitboard& blockerBoard : blockerConfigs) {
				Bitboard moves = createOrthogonalMovement(square, blockerBoard);
				movementBoards.push_back(moves);
			}

			// Add all orthogonal movement bitboards for each blocker permutation
			BitMagics::addOrthogonalMoves(square, blockerConfigs, movementBoards);
		}
	}

	// Computes all orthogonal moves
	static void precomputeDiagonalMoves() {
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			// Generate all potential blocker squares - excluding esge squares
			// since edge piece cannot block anything behind them
			vector<Bitboard> blockerConfigs = precomputeDiagonalMove(square);
			// Need blockerConfigs.size() movement boards to accomodate each permutation
			// of blockers
			vector<Bitboard> movementBoards;
			movementBoards.reserve(blockerConfigs.size());

			// For each blockerboard, generate a movement bitboard for that permutation
			// of blockers
			for (Bitboard& blockerBoard : blockerConfigs) {
				Bitboard moves = createDiagonalMovement(square, blockerBoard);
				movementBoards.push_back(moves);
			}

			// Add all orthogonal movement bitboards for each blocker permutation
			BitMagics::addDiagonalMoves(square, blockerConfigs, movementBoards);
		}
	}

	// Computes movement bitboards for each piece type
	static void precomputeMoves() {
		precomputeOrthogonalMoves();
		precomputeDiagonalMoves();
		precomputeKnightMoves();
		precomputeKingMoves();
		precomputePawnMoves();
	}

	static void initOrthogonalMasks() {
		// Orthogonal masks
		for (int i = 0; i < 8; ++i) {
			rowMasks[i] = rowMask << (i * 8);
			columnMasks[i] = columnMask << i;
		}
	}

	static void initDiagonalMasks() {
		// Diagonal masks
		for (int i = 0; i < 15; i++) {
			initForwardMask(i);
			initBackwardMask(i);
		}
	}

	static void initBlockerMasks() {
		// Blocker Masks
		for (Square square = START_SQUARE; square < SQUARES_NB; ++square) {
			createOrthogonalMask(square);
			createDiagonalMask(square);
		}
	}

	// Initialises all masks
	static void initMasks() {
		initBlockerMasks();
		initOrthogonalMasks();
		initDiagonalMasks();
		initPassedPawnMasks();
		initIslandMasks();
		initShieldMasks();
		initLinesBB();
	}

	void initBitboards() {
		initDirectionDistances();
		
		initMasks(); // Compute necessary masks
		// Magics initialisation
		BitMagics::initMagics();
		precomputeMoves(); // Compute magic bitboard (hashtable for orthogonal and diagonal moves)
	}

}