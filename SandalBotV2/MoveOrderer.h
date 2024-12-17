#pragma once

#ifndef MOVEORDERER_H
#define MOVEORDERER_H

#include "Board.h"
#include "PieceEvaluations.h"
#include "MoveGen.h"

class Searcher;

class MoveOrderer {
private:
	struct Killer {
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

	Board* board = nullptr;
	MoveGen* generator = nullptr;
	Searcher* searcher = nullptr;

	const int killerValue = 1000;
	Killer killerMoves[32];
public:
	MoveOrderer();
	MoveOrderer(Board* board, MoveGen* gen, Searcher* searcher);
	~MoveOrderer();
	void order(Move moves[], Move& bestMove, int numMoves, int depth, bool firstMove = false, bool qSearch = false);
	static void quickSort(Move moves[], int moveVals[], int start, int end);
	void addKiller(int depth, Move& move);
};

#endif