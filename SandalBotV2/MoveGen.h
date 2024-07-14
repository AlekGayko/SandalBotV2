#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Board.h"
#include "MovePrecomputation.h"
#include "CoordHelper.h"
#include "Piece.h"

class MoveGen {
private:
	Board* board = nullptr;
	MovePrecomputation preComp;
	static constexpr int directions[8] = { -8, 8, -1, 1, -7, 7, -9, 9 };
	static constexpr int knightDirections[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
	static constexpr int minOrthogonal = 0;
	static constexpr int maxOrthogonal = 4;
	static constexpr int minDiagonal = 4;
	static constexpr int maxDiagonal = 8;
	int currentMoves = 0;
	int currentColor;
public:
	const int maxMoves = 218;

	MoveGen();
	MoveGen(Board* board);
	int generateMoves(Move moves[], bool isWhite);
	void generateOrthogonalMoves(Move moves[], int startSquare);
	void generateDiagonalMoves(Move moves[], int startSquare);
	void generateKnightMoves(Move moves[], int startSquare);
	void generateKingMoves(Move moves[], int startSquare);
	void generatePawnMoves(Move moves[], int startSquare);
};

#endif
