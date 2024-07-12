#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Board.h"
#include "MovePrecomputation.h"

class MoveGen {
private:
	Board* board = nullptr;
public:
	int maxMoves = 218;

	inline MoveGen(Board* board);
	inline void generateMoves(int moves[]);
	inline void generateOrthogonalMoves(int moves[], int startSquare, int pieceType);
	inline void generateDiagonalMoves(int moves[], int startSquare, int pieceType);
	inline void generateKnightMoves(int moves[], int startSquare);
	inline void generateKingMoves(int moves[], int startSquare);
	inline void generatePawnMoves(int moves[], int startSquare);
};

#endif
