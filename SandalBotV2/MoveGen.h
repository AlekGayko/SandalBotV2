#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Board.h"
#include "MovePrecomputation.h"

class MoveGen {
private:
	Board* board = nullptr;
	static constexpr int directions[8] = { -8, 8, -1, 1, -7, 7, -9, 9 };
	static constexpr int minOrthogonal = 0;
	static constexpr int maxOrthogonal = 4;
	static constexpr int minDiagonal = 4;
	static constexpr int maxDiagonal = 8;
	static constexpr int minKnight = 8;
	static constexpr int maxKnight = 12;
	int currentMoves = 0;
	int currentColor;
public:
	int maxMoves = 218;

	inline MoveGen(Board* board);
	inline void generateMoves(Move moves[]);
	inline void generateOrthogonalMoves(Move moves[], int startSquare);
	inline void generateDiagonalMoves(Move moves[], int startSquare);
	inline void generateKnightMoves(Move moves[], int startSquare);
	inline void generateKingMoves(Move moves[], int startSquare);
	inline void generatePawnMoves(Move moves[], int startSquare);
};

#endif
