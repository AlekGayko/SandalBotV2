#ifndef BITBOARDUTILITY_H
#define BITBOARDUTILITY_H

#include "CoordHelper.h"
#include "Magics.h"
#include "Move.h"
#include "Types.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

#if defined(__GNUC__)
	#include <x86intrin.h>
#elif defined(_MSC_VER) 
	#include <intrin.h>
#endif

// BitBoardUtility provides utility functions for bitboards
namespace SandalBot {

	// Mask of single row and column
	constexpr Bitboard rowMask{ 0b11111111ULL };
	constexpr Bitboard columnMask{ 0x0101010101010101ULL };
	// Mask for single row and column for blockers, but without edges (since edges cannot block behind them)
	constexpr Bitboard blockerRowMask{ 0b01111110ULL };
	constexpr Bitboard blockerColumnMask{ 0x0001010101010100ULL };

	extern uint8_t distances[SQUARES_NB][SQUARES_NB]; // Stores distance between any two coordinates
	// King and knight movement bitboards
	// Pawn shield bitboard masks for king
	extern Bitboard pawnShieldMask[COLOR_NB][SQUARES_NB];
	// Bitboards for attack zone around king
	extern Bitboard kingAttackZone[COLOR_NB][SQUARES_NB];
	// Bitboard attack zone for either white or black king
	extern Bitboard kingUnbiasAttackZone[SQUARES_NB];
	// Movement bitboards for attack moves for white and black pawns
	extern Bitboard pawnAttackMoves[COLOR_NB][SQUARES_NB];
	// Bitboard masks for white and black pawns for passed pawns.
	// Masks pawn's column and adjacent columns in front of pawn
	extern Bitboard passedPawnMasks[COLOR_NB][SQUARES_NB];
	// Bitboard masks for each column for pawn islands. (adjacent column masks)
	extern Bitboard pawnIslandMasks[ROW_NB];
	// Bitboard masks for all orthogonal and diagonal directions
	extern Bitboard blockerOrthogonalMasks[SQUARES_NB];
	extern Bitboard blockerDiagonalMasks[SQUARES_NB];
	// Masks for all rows and columns
	extern Bitboard rowMasks[ROW_NB];
	extern Bitboard columnMasks[COL_NB];
	extern Bitboard forwardDiagonalMasks[15]; // 45 degree diagonals
	extern Bitboard backwardDiagonalMasks[15]; // -45 degree diagonals

	extern Bitboard movementBoards[PIECE_TYPE_NB][SQUARES_NB];

	extern Bitboard linesBB[SQUARES_NB][SQUARES_NB];
	extern Bitboard linesBetweenBB[SQUARES_NB][SQUARES_NB];

	// Returns a bitmask of bounding box between two squares
	Bitboard boxMask(Square sq1, Square sq2);

	inline Bitboard getBit(Bitboard bitboard, int index) {
		return bitboard & (1ULL << index);
	}
	// Prints the individual bits of the bitboard
	// the printed output is in form of 8x8 board
	inline void printBB(Bitboard bitboard) {
		for (Row row = ROW_8; row <= ROW_1; ++row) {
			for (Column col = COL_A; col <= COL_H; ++col) {
				Square square = Square(row * 8 + col);
				int bit = getBit(bitboard, square) ? 1 : 0;
				std::cout << " " << bit;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	// 'pops' the least significant bit from a bitboard and returns the index of that bit
	inline Square popLSB(Bitboard& bitBoard) {
		// index of LSB
		Square trailingZeroes = static_cast<Square>(_tzcnt_u64(bitBoard));

		bitBoard &= bitBoard - 1ULL; // Use bit manipulation to remove LSB
		return trailingZeroes;
	}

	inline Square LSB(Bitboard bitBoard) {
		return static_cast<Square>(_tzcnt_u64(bitBoard));
	}

	enum DistIndex : int {
		NORTH_IDX = 0,
		EAST_IDX,
		SOUTH_IDX,
		WEST_IDX,
		NORTH_WEST_IDX,
		NORTH_EAST_IDX,
		SOUTH_EAST_IDX,
		SOUTH_WEST_IDX,

		DISTINDEX_NB,
		DISTINDEX_START = NORTH_IDX,

		START_ORTH = NORTH_IDX,
		END_ORTH = NORTH_WEST_IDX,
		START_DIAG = NORTH_WEST_IDX,
		END_DIAG = DISTINDEX_NB
	};

	inline DistIndex& operator++(DistIndex& d) { return d = DistIndex(int(d) + 1); }

	struct dirDist {
		// Distances between piece and sides of board
		int slideDistances[DISTINDEX_NB]{};
		bool knightSquares[DISTINDEX_NB]{};
		dirDist() {}
		dirDist(int top, int left, int right, int bottom);
	};

	constexpr Direction slideDirections[DISTINDEX_NB]{
		NORTH, EAST, SOUTH, WEST, NORTH_WEST, NORTH_EAST, SOUTH_EAST, SOUTH_WEST
	};
	// Knight directions
	constexpr Direction knightDirections[DISTINDEX_NB]{
		Direction(2 * NORTH + WEST),
		Direction(2 * NORTH + EAST),
		Direction(NORTH + 2 * EAST),
		Direction(SOUTH + 2 * EAST),
		Direction(2 * SOUTH + EAST),
		Direction(2 * SOUTH + WEST),
		Direction(SOUTH + WEST),
		Direction(NORTH + 2 * WEST)
	};

	inline Bitboard getLineBB(Square sq1, Square sq2) { return linesBB[sq1][sq2]; }
	inline Bitboard getLineBetweenBB(Square sq1, Square sq2) { return linesBetweenBB[sq1][sq2]; }

	inline Bitboard getForwardMask(const Square square) { return forwardDiagonalMasks[toRow(square) + toCol(square)]; }
	inline Bitboard getBackwardMask(const Square square) { return backwardDiagonalMasks[7 + toRow(square) - toCol(square)]; }
	inline Bitboard getRowMask(const Square square) { return rowMask << (toRow(square) * 8); }
	inline Bitboard getColMask(const Square square) { return columnMask << toCol(square); }
	inline Bitboard getBlockerOrthogonalMask(const Square square) { return blockerOrthogonalMasks[square]; }
	inline Bitboard getBlockerDiagonalMask(const Square square) { return blockerDiagonalMasks[square]; }
	inline Bitboard getOrthMovementBoard(const Square square, const Bitboard blockerBoard) { return BitMagics::getOrthogonalMovement(square, blockerBoard); }
	inline Bitboard getDiagMovementBoard(const Square square, const Bitboard blockerBoard) { return BitMagics::getDiagonalMovement(square, blockerBoard); }

	template <Color Us>
	inline Bitboard getPawnAttackMoves(const Square square) { return pawnAttackMoves[Us][square]; }
	template <Color Us>
	inline Bitboard getPassedPawnMask(const Square square) { return passedPawnMasks[Us][square]; }
	inline Bitboard getPawnIslandMask(const int column) { return pawnIslandMasks[column]; }
	template <Color Us>
	inline Bitboard getShieldMask(const Square square) { return pawnShieldMask[Us][square]; }
	inline uint8_t getDistance(const Square square1, const Square square2) { return distances[square1][square2]; }
	template <Color Us>
	inline Bitboard getKingAttackSquare(const Square square) { return kingAttackZone[Us][square]; }
	inline Bitboard getUnbiasKingAttackZone(const Square square) { return kingUnbiasAttackZone[square]; }

	template <PieceType Type>
	inline Bitboard getMovementBoard(Square sq, Bitboard allPieces) {
		return movementBoards[Type][sq];
	}

	template <>
	inline Bitboard getMovementBoard<ROOK>(Square sq, Bitboard allPieces) {
		Bitboard orthogonalBlockers = allPieces & getBlockerOrthogonalMask(sq);
		return getOrthMovementBoard(sq, orthogonalBlockers);
	}

	template <>
	inline Bitboard getMovementBoard<BISHOP>(Square sq, Bitboard allPieces) {
		Bitboard diagonalBlockers = allPieces & getBlockerDiagonalMask(sq);
		return getDiagMovementBoard(sq, diagonalBlockers);
	}

	template <>
	inline Bitboard getMovementBoard<QUEEN>(Square sq, Bitboard allPieces) {
		return getMovementBoard<ROOK>(sq, allPieces) | getMovementBoard<BISHOP>(sq, allPieces);
	}

	void initBitboards();

}

#endif // !BITBOARDUTILITY_H
