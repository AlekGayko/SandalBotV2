#pragma once

#ifndef MOVEPRECOMPUTATION_H
#define MOVEPRECOMPUTATION_H

#include "Move.h"
#include "CoordHelper.h"

class PrecomputedMagics;

#include <vector>

class MovePrecomputation {
	static constexpr uint64_t rowMask = 0b11111111ULL;
	static constexpr uint64_t columnMask = 0x0101010101010101ULL;

	static constexpr uint64_t blockerRowMask = 0b01111110ULL;
	static constexpr uint64_t blockerColumnMask = 0x0001010101010100ULL;

	const int slideDirections[8] = { -8, 1, 8, -1, -9, -7, 9, 7 };
	const int knightDirections[8] = { -17, -15, -6, 10, 17, 15, 6, -10 };

	const int startOrthogonal = 0;
	const int endOrthogonal = 4;
	const int startDiagonal = 4;
	const int endDiagonal = 8;
	
	PrecomputedMagics* magics = nullptr;

	uint64_t knightMoves[64];
	uint64_t kingMoves[64];

	uint64_t whitePawnMoves[64];
	uint64_t whitePawnAttackMoves[64];
	uint64_t blackPawnMoves[64];
	uint64_t blackPawnAttackMoves[64];

	uint64_t blockerOrthogonalMasks[64];
	uint64_t blockerDiagonalMasks[64];

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
public:
	struct dirDist {
		// Distances between piece and sides of board
		int top;
		int left;
		int right;
		int bottom;
		int direction[8];
		bool knightSquares[8];
		int minVertical;
		int minHorizontal;
		dirDist();
		dirDist(int top, int left, int right, int bottom);
	};

	dirDist directionDistances[64];
	

	MovePrecomputation();
	~MovePrecomputation();

	uint64_t getForwardMask(const int& square);
	uint64_t getBackwardMask(const int& square);
	uint64_t getRowMask(const int& square);
	uint64_t getColMask(const int& square);
	uint64_t getBlockerOrthogonalMask(const int& square);
	uint64_t getBlockerDiagonalMask(const int& square);
	uint64_t getOrthMovementBoard(const int& square, const uint64_t& blockerBoard);
	uint64_t getDiagMovementBoard(const int& square, const uint64_t& blockerBoard);
	uint64_t getKnightBoard(const int& square);
	uint64_t getKingMoves(const int& square);
	uint64_t getPawnMoves(const int& square, const uint64_t& blockerBoard, const int& color);
	uint64_t getPawnAttackMoves(const int& square, const int& color);
	uint64_t getDirectionMask(const int& square1, const int& square2);
};

#endif // !MOVEPRECOMPUTATION_H