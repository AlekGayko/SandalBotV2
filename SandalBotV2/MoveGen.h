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

	const Coord orthogonalDirections[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	const Coord diagonalDirections[4] = { { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } };
	const Coord knightDirections[8] = { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } };
	const Coord whitePawnDirection = { -1, 0 };
	const Coord blackPawnDirection = { 1, 0 };
	const Coord whitePawnAttacks[2] = { { -1, -1 }, { -1, 1 } };
	const Coord blackPawnAttacks[2] = { { 1, -1 }, { 1, 1 } };

	const int promotionFlags[4] = { Move::promoteToQueenFlag, Move::promoteToRookFlag, Move::promoteToKnightFlag, Move::promoteToBishopFlag };

	const int startingKingSquares[2] = { 4, 60 };
	const int shortCastleKingSquares[2] = { 6, 62 };
	const int longCastleKingSquares[2] = { 2, 58 };
	const int shortCastleRookSquares[2] = { 7, 63 };
	const int longCastleRookSquares[2] = { 0, 56 };

	int currentMoves;
	int currentColor;
	int opposingColor;
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
	void enPassantMoves(Move moves[], int targetSquare, int startSquare);
	void promotionMoves(Move moves[], int targetSquare, int startSquare);
	void castlingMoves(Move moves[], int startSquare);
};

#endif
