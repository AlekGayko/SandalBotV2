#pragma once

#ifndef MOVEORDERER_H
#define MOVEORDERER_H

#include "Board.h"
#include "MoveGen.h"
#include "PieceEvaluations.h"

class MoveOrderer {
private:
	Board* board = nullptr;
	MoveGen* generator = nullptr;
	void quickSort(Move moves[]);
public:
	MoveOrderer();
	MoveOrderer(Board* board, MoveGen* gen);
	void order(Move moves[], int numMoves);
	static void quickSort(Move moves[], int moveVals[], int start, int end);
};

#endif