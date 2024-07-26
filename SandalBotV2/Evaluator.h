#pragma once

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "Board.h"
#include "PieceEvaluations.h"

class Evaluator {
private:
	int colorStart[2] = { 63, 0 };
	int colorDirection[2] = { -1, 1 };
public:
	static const int checkMateScore = -100000;

	Evaluator();
	int Evaluate(Board* board);
};

#endif