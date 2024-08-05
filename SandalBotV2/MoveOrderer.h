#pragma once

#ifndef MOVEORDERER_H
#define MOVEORDERER_H

#include "Board.h"
#include "PieceEvaluations.h"
#include "MoveGen.h"

class Searcher;

class MoveOrderer {
private:
	Board* board = nullptr;
	MoveGen* generator = nullptr;
	Searcher* searcher = nullptr;
public:
	MoveOrderer();
	MoveOrderer(Board* board, MoveGen* gen, Searcher* searcher);
	~MoveOrderer();
	void order(Move moves[], Move bestMove, int numMoves, bool firstMove = false);
	static void quickSort(Move moves[], int moveVals[], int start, int end);
};

#endif