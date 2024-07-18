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
	/*
	const Coord orthogonalDirections[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	const Coord diagonalDirections[4] = { { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } };
	const Coord kingDirections[8] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } };
	const Coord knightDirections[8] = { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } };
	const Coord whitePawnDirection = { -1, 0 };
	const Coord blackPawnDirection = { 1, 0 };
	const Coord whitePawnAttacks[2] = { { -1, -1 }, { -1, 1 } };
	const Coord blackPawnAttacks[2] = { { 1, -1 }, { 1, 1 } };
	*/

	const int slideDirections[8] = { -8, 1, 8, -1, -9, -7, 9, 7 };
	const int knightDirections[8] = { -17, -15, -6, 10, 17, 15, 6, -10 };
	const int whitePawnDirection = -8;
	const int blackPawnDirection = 8;
	const int whitePawnAttackDirections[2] = { -9, -7 };
	const int blackPawnAttackDirections[2] = { 7, 9 };

	const int pieces[6] = { Piece::king, Piece::queen, Piece::rook, Piece::knight, Piece::bishop, Piece::pawn };

	const int startOrthogonal = 0;
	const int endOrthogonal = 4;
	const int startDiagonal = 4;
	const int endDiagonal = 8;

	const int promotionFlags[4] = { Move::promoteToQueenFlag, Move::promoteToRookFlag, Move::promoteToKnightFlag, Move::promoteToBishopFlag };

	const int startingKingSquares[2] = { 4, 60 };
	const int shortCastleKingSquares[2] = { 6, 62 };
	const int longCastleKingSquares[2] = { 2, 58 };
	const int shortCastleRookSquares[2] = { 7, 63 };
	const int longCastleRookSquares[2] = { 0, 56 };

	int currentMoves;
	int colorIndex;
	int enemyColorIndex;
	int currentColor;
	int opposingColor;
	bool doubleCheck;
	PieceList* friendlyPieceLists;
	PieceList* enemyPieceLists;
	PieceList orthogonalSliders[2];
	PieceList diagonalSliders[2];

	bool isCheck;
	bool whiteTurn;
	int enPassantFile;
	int castlingRights;
	int fiftyMoveCounter;
	int moveCounter;


public:
	const int maxMoves = 218;

	MoveGen();
	MoveGen(Board* board);
	int generateMoves(Move moves[]);
	void initVariables();
	void generateSlideMoves(Move moves[]);
	void generateOrthogonalMoves(Move moves[]);
	void generateDiagonalMoves(Move moves[]);
	void generateKnightMoves(Move moves[]);
	void generateKingMoves(Move moves[]);
	void generatePawnMoves(Move moves[]);
	void enPassantMoves(Move moves[], int targetSquare, int startSquare);
	void promotionMoves(Move moves[], int targetSquare, int startSquare);
	void castlingMoves(Move moves[], int startSquare);

	void generateCheckData();
};

#endif
