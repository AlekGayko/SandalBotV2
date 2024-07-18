#pragma once

#ifndef BOARD_H
#define BOARD_H

#include "BoardHistory.h"
#include "BoardState.h"
#include "CoordHelper.h"
#include "Move.h"
#include "PieceList.h"

class FEN;

class Board {
private:
	
public:
	BoardState state;
	int squares[64];

	PieceList pieceLists[2][7]; // index [i][0] is redundant since 0 = empty
	//int kings[2];
	BoardHistory history;

	static const int blackIndex = 0;
	static const int whiteIndex = 1;

	int testMoves = 0;

	Board();
	void loadPieceLists();
	void loadPosition(std::string fen);
	void makeMove(Move& move);
	void makeEnPassantChanges(Move& move);
	void makeCastlingChanges(Move& move);
	void makePromotionChanges(Move& move);
	void unMakeMove(Move& move);
	void undoEnPassantChanges(Move& move);
	void undoCastlingChanges(Move& move);
	void undoPromotionChanges(Move& move);
	std::string printBoard();
};

#endif // !BOARD_H
