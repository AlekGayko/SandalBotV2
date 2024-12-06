#pragma once

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "Board.h"
#include "PieceEvaluations.h"
#include "MovePrecomputation.h"

class Evaluator {
private:
	MovePrecomputation* precomputation;
	Board* board;
	const float endgameRequiredPieces = 7;
	// Center manhattan-distance from https://www.chessprogramming.org/Center_Manhattan-Distance
	const unsigned char arrCenterManhattanDistance[64] = {
		6, 5, 4, 3, 3, 4, 5, 6,
		5, 4, 3, 2, 2, 3, 4, 5,
		4, 3, 2, 1, 1, 2, 3, 4,
		3, 2, 1, 0, 0, 1, 2, 3,
		3, 2, 1, 0, 0, 1, 2, 3,
		4, 3, 2, 1, 1, 2, 3, 4,
		5, 4, 3, 2, 2, 3, 4, 5,
		6, 5, 4, 3, 3, 4, 5, 6
	};

	const unsigned char passedPawnBonus[7] = { 0, 90, 60, 40, 25, 15, 15 };
	const unsigned char pawnIslandPenalty = 30;

	float endGameWeight;
	float numMajorMinorPieces;

	int friendlyColor;
	int enemyColor;
	int friendlyMajorMinorPieces;
	int enemyMajorMinorPieces;
	int friendlyKingSquare;
	int enemyKingSquare;
	int maximisingIndex;
	int minimisingIndex;
	int maximisingSide;
	int evaluation;

	uint64_t friendlyBoard;
	uint64_t enemyBoard;

	void initBlackSquares();
	void initVariables();
	void calculateEndgameWeight();
	int staticPieceEvaluations();
	int kingDist();
	int kingTropism();
	int pawnShieldEvaluation();
	int passedPawnEvaluation();
	int pawnIslandEvaluation();
public:
	static int blackEvalSquare[64];
	static int colorStart[2];
	static const int checkMateScore = 100000;
	static const int drawScore = 0;
	static const int cancelledScore = 0;

	Evaluator();
	Evaluator(Board* board, MovePrecomputation* precomputation);
	int Evaluate();
};

#endif