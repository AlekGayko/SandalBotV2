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
	const float tropismWeightings[7] = { 0.f, 0.2f, 0.5f, 0.5f, 1.f, 2.f, 0.f };
	const unsigned char pawnIslandPenalty = 30;
	const unsigned char pawnShieldColumnPenalty = 40;
	const unsigned char pawnShieldUndefendedPenalty = 20;

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
	int staticPieceEvaluation(PieceList* pieceList, bool useBlackSquares=false);
	int kingDist();
	int kingTropismEvaluations();
	int kingTropism(const int& kingSquare, PieceList* enemyList);
	int pawnShieldEvaluations();
	int pawnShieldEvaluation(const int& square, uint64_t& pawns);
	int passedPawnEvaluations();
	int passedPawnEvaluation(PieceList& pieceList, uint64_t& pawns, uint64_t& opposingPawns, const int& color);
	int pawnIslandEvaluations();
	int pawnIslandEvaluation(PieceList& pieceList, uint64_t& pawns);
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