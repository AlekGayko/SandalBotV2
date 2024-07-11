#pragma once
#ifndef BOARD_H
#define BOARD_H

#include "Piece.h"
#include "PieceList.h"
#include "Move.h"
#include "BoardState.h"

class Board {
private:
	BoardState state;

	PieceList* kings = nullptr;
	PieceList* queens = nullptr;
	PieceList* rooks = nullptr;
	PieceList* bishop = nullptr;
	PieceList* knights = nullptr;
	PieceList* pawns = nullptr;
public:
	Board();
	void MakeMove(Move move);
	void UnMakeMove(Move move);
};

#endif // !BOARD_H
