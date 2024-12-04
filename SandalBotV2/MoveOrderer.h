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

	const int killerValue = 1000;
public:
	struct Killers {
		Move moveA;
		Move moveB;
		void add(Move& move) {
			if (move.moveValue != moveA.moveValue) {
				moveB = moveA;
				moveA = move;
			}
		}
		bool match(Move& move) {
			return move.moveValue == moveA.moveValue || move.moveValue == moveB.moveValue;
		}
	};
	Killers killerMoves[32];
	MoveOrderer();
	MoveOrderer(Board* board, MoveGen* gen, Searcher* searcher);
	~MoveOrderer();
	void order(Move moves[], Move& bestMove, int numMoves, int depth, bool firstMove = false, bool qSearch = false);
	static void quickSort(Move moves[], int moveVals[], int start, int end);
};

#endif