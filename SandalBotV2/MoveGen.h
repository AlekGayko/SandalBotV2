#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Board.h"
#include "Piece.h"
#include "MovePrecomputation.h"

class MoveGen {
private:
	Board board;
public:
	int maxMoves = 218;

	MoveGen(Board board);

	void generateMoves(int moves[]);
	void generateOrthogonalMoves(int moves[], int startSquare, int pieceType);
	void generateDiagonalMoves(int moves[], int startSquare, int pieceType);
	void generateKnightMoves(int moves[], int startSquare);
	void generateKingMoves(int moves[], int startSquare);
	void generatePawnMoves(int moves[], int startSquare);
};

#endif
