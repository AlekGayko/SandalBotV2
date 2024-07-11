#pragma once

#ifndef BOARD_H
#define BOARD_H

#include "Piece.h"
#include "PieceList.h"
#include "Move.h"
#include "BoardState.h"
#include "FEN.h"

class Board {
private:
	
public:
	BoardState state;
	int squares[64];

	PieceList* pieceLists[6];
	int kings[2];
	PieceList queens[2];
	PieceList rooks[2];
	PieceList bishop[2];
	PieceList knights[2];
	PieceList pawns[2];

	const int blackIndex = 0;
	const int whiteIndex = 1;

	int numMoves = 0;

	Board();
	void loadPosition(std::string fen);
	void MakeMove(Move move);
	void UnMakeMove(Move move);
};

#endif // !BOARD_H
