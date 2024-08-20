#pragma once

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "Board.h"
#include "PieceEvaluations.h"

class Evaluator {
private:
	int endGameWeight;
public:
	static int colorStart[2];
	static const int checkMateScore = 100000;
	static const int drawScore = 0;
	static const int cancelledScore = 0;
	int maximisingSide;
	static int blackEvalSquare[64];

	Evaluator();
	int Evaluate(Board* board);
	int kingDist(Board* board);
	void initBlackSquares();
};

#endif