#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "BitBoardUtility.h"
#include "Board.h"
#include "MovePrecomputation.h"
#include "CoordHelper.h"
#include "Piece.h"

struct PerftResults {
	int captures;
	int enPassants;
	int castles;
	int promotions;
	int checks;
	int discoveryChecks;
	int doubleChecks;
	int checkmates;
	int stalemates;
	void reset();
	friend std::ostream& operator<<(std::ostream& os, const PerftResults& res);
};

class MoveGen {
private:
	Board* board = nullptr;
	MovePrecomputation preComp;

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

	unsigned long long int currentMoves;
	int colorIndex;
	int enemyColorIndex;
	int currentColor;
	int opposingColor;
	bool doubleCheck;
	int friendlyKingSquare;
	int* squares = nullptr;
	PieceList* friendlyPieceLists;
	PieceList* enemyPieceLists;
	PieceList orthogonalSliders[2];
	PieceList diagonalSliders[2];

	PieceList enemyOrthogonalSliders[2];
	PieceList enemyDiagonalSliders[2];

	PieceList pawns;

	bool whiteTurn;
	int enPassantSquare;
	int castlingRights;
	int fiftyMoveCounter;
	int moveCounter;

	unsigned long long int opponentAttacks;
	unsigned long long int checkBB;
	unsigned long long int checkRayBB;

public:
	static const int startingKingSquares[2];
	static const int shortCastleKingSquares[2];
	static const int longCastleKingSquares[2];
	static const int shortCastleRookSquares[2];
	static const int longCastleRookSquares[2];
	static const int shortCastleRookSpawn[2];
	static const int longCastleRookSpawn[2];

	PerftResults perftRes;
	const int maxMoves = 218;
	bool isCheck;

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

	void generateKnightAttackData();
	void generatePawnAttackData();
	void generateKingAttackData();
	void generateSlideAttackData();
	void generateAttackData();
	void generateCheckData();

	bool enPassantPin(int friendlyPawnSquare, int enemyPawnSquare);

	void updateResults(Move moves[]);
};

#endif
