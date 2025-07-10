#ifndef MOVEPRECOMPUTATION_H
#define MOVEPRECOMPUTATION_H

#include "Move.h"
#include "CoordHelper.h"
#include "PrecomputedMagics.h"

#include <vector>

namespace SandalBot {

	// MovePrecomputation provides utility data for move generation, evaluation and more.
	// Data is computed on instantiation to avoid recomputation
	class MovePrecomputation {
		public:
		struct dirDist {
			// Distances between piece and sides of board
			int top{};
			int left{};
			int right{};
			int bottom{};
			int direction[8]{};
			bool knightSquares[8]{};
			int minVertical{};
			int minHorizontal{};
			dirDist() {}
			dirDist(int top, int left, int right, int bottom);
		};

		dirDist directionDistances[64]; // Provides distance to edges


		MovePrecomputation();

		uint64_t getForwardMask(const int square) const { return forwardDiagonalMasks[(square / 8) + (square % 8)]; }
		uint64_t getBackwardMask(const int square) const { return backwardDiagonalMasks[7 + (square / 8) - (square % 8)]; }
		uint64_t getRowMask(const int square) const { return rowMask << ((square / 8) * 8); }
		uint64_t getColMask(const int square) const { return columnMask << (square % 8); }
		uint64_t getBlockerOrthogonalMask(const int square) const { return blockerOrthogonalMasks[square]; }
		uint64_t getBlockerDiagonalMask(const int square) const { return blockerDiagonalMasks[square]; }
		uint64_t getOrthMovementBoard(const int square, const uint64_t blockerBoard) const { return magics.getOrthogonalMovement(square, blockerBoard); }
		uint64_t getDiagMovementBoard(const int square, const uint64_t blockerBoard) const { return magics.getDiagonalMovement(square, blockerBoard); }
		uint64_t getKnightBoard(const int square) const { return knightMoves[square]; }
		uint64_t getKingMoves(const int square) const { return kingMoves[square]; }
		uint64_t getPawnAttackMoves(const int square, const int color) const { return color == Piece::white ? whitePawnAttackMoves[square] : blackPawnAttackMoves[square]; }
		uint64_t getPassedPawnMask(const int square, const int color) const { return color == Piece::white ? whitePassedPawnMasks[square] : blackPassedPawnMasks[square]; }
		uint64_t getPawnIslandMask(const int column) const { return pawnIslandMasks[column]; }
		uint64_t getShieldMask(const int square, const int color) const { return color == Piece::white ? whitePawnShieldMask[square] : blackPawnShieldMask[square]; }
		uint64_t getDirectionMask(const int square1, const int square2);
		uint8_t getDistance(const int square1, const int square2) const { return distances[square1][square2]; }
		uint64_t getKingAttackSquare(const int square, const int color) const { return color == Piece::white ? whiteKingAttackZone[square] : blackKingAttackZone[square]; }
		uint64_t getUnbiasKingAttackZone(const int square) const { return kingUnbiasAttackZone[square]; }
	private:
		// Mask of single row and column
		static constexpr uint64_t rowMask{ 0b11111111ULL };
		static constexpr uint64_t columnMask{ 0x0101010101010101ULL };
		// Mask for single row and column for blockers, but without edges (since edges cannot block behind them)
		static constexpr uint64_t blockerRowMask{ 0b01111110ULL };
		static constexpr uint64_t blockerColumnMask{ 0x0001010101010100ULL };
		// Directions for diagonals and orthogonals
		const int slideDirections[8] { -8, 1, 8, -1, -9, -7, 9, 7 };
		// Knight directions
		const int knightDirections[8] { -17, -15, -6, 10, 17, 15, 6, -10 };
		// Indexes for above arrays
		const int startOrthogonal{ 0 };
		const int endOrthogonal{ 4 };
		const int startDiagonal{ 4 };
		const int endDiagonal{ 8 };
		// Stores hashtable for magic bitboards
		PrecomputedMagics magics{};

		uint8_t distances[64][64]; // Stores distance between any two coordinates
		// King and knight movement bitboards
		uint64_t knightMoves[64];
		uint64_t kingMoves[64];
		// Pawn shield bitboard masks for king
		uint64_t whitePawnShieldMask[64];
		uint64_t blackPawnShieldMask[64];
		// Bitboards for attack zone around king
		uint64_t whiteKingAttackZone[64];
		uint64_t blackKingAttackZone[64];
		// Bitboard attack zone for either white or black king
		uint64_t kingUnbiasAttackZone[64];
		// Movement bitboards for attack moves for white and black pawns
		uint64_t whitePawnAttackMoves[64];
		uint64_t blackPawnAttackMoves[64];
		// Bitboard masks for white and black pawns for passed pawns.
		// Masks pawn's column and adjacent columns in front of pawn
		uint64_t whitePassedPawnMasks[64];
		uint64_t blackPassedPawnMasks[64];
		// Bitboard masks for each column for pawn islands. (adjacent column masks)
		uint64_t pawnIslandMasks[8];
		// Bitboard masks for all orthogonal and diagonal directions
		uint64_t blockerOrthogonalMasks[64];
		uint64_t blockerDiagonalMasks[64];
		// Masks for all rows and columns
		uint64_t rowMasks[8];
		uint64_t columnMasks[8];
		uint64_t forwardDiagonalMasks[15]; // 45 degree diagonals
		uint64_t backwardDiagonalMasks[15]; // -45 degree diagonals


		uint64_t directionMasks[512]; // 64 (number of squares) * 8 (number of directions)
		int differenceDivisibles[128]; // Lord why am I making so many tables

		void initMasks();
		void initForwardMask(int constant);
		void initBackwardMask(int constant);
		void initDirectionMasks();
		void initPassedPawnMasks();
		void initIslandMasks();
		void initShieldMasks();
		void initDistances();
		void initKingAttackSquares();
		void precomputeMoves();
		void precomputeOrthogonalMoves();
		void precomputeDiagonalMoves();
		void precomputeKnightMoves();
		void precomputeKingMoves();
		void precomputePawnMoves();
		std::vector<uint64_t> precomputeOrthogonalMove(int square);
		std::vector<uint64_t> precomputeDiagonalMove(int square);
		std::vector<uint64_t> makeConfigs(std::vector<int>& moveSquares);
		uint64_t createOrthogonalMask(int square);
		uint64_t createDiagonalMask(int square);
		uint64_t createOrthogonalMovement(int square, uint64_t blockerBoard);
		uint64_t createDiagonalMovement(int square, uint64_t blockerBoard);
		uint64_t createMovement(int square, uint64_t blockerBoard, int start, int end);
	};

}

#endif // !MOVEPRECOMPUTATION_H