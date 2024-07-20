#pragma once

#ifndef BOARD_H
#define BOARD_H

#include "BoardHistory.h"
#include "BoardState.h"
#include "CoordHelper.h"
#include "Move.h"
#include "PieceList.h"

#include <stack>

class FEN;

class Board {
private:
	
public:
	BoardState state;
	int squares[64];

	PieceList pieceLists[2][7]; // index [i][0] is redundant since 0 = empty
	//int kings[2];

	BoardHistory history;
	std::stack<BoardState> boardStateHistory;

	// Bitboards
	unsigned long long int bitBoards[7];

	unsigned long long int allPieces;
	unsigned long long int whitePieces;
	unsigned long long int blackPieces;
	unsigned long long int orthogonalPieces;
	unsigned long long int diagonalPieces;
	unsigned long long int pawns;
	unsigned long long int knights;

	static const int blackIndex = 0;
	static const int whiteIndex = 1;

	int testMoves = 0;

	Board();
	~Board();
	void loadPieceLists();
	void loadBitBoards();
	void loadPosition(std::string fen);
	void makeMove(Move& move);
	void makeEnPassantChanges(Move& move);
	void makeCastlingChanges(Move& move);
	void makePromotionChanges(Move& move);
	void unMakeMove(Move& move);
	void undoEnPassantChanges(Move& move);
	void undoCastlingChanges(Move& move);
	void undoPromotionChanges(Move& move);
	void updateBitBoards(Move& move, int pieceType, int takenPiece);
	void undoBitBoards(Move& move, int pieceType, int takenPiece);
	std::string printBoard();
};

#endif // !BOARD_H
