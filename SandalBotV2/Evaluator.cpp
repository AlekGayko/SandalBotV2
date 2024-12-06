#include "Evaluator.h"
#include "BitBoardUtility.h"

#include <iostream>

using namespace std;

int Evaluator::colorStart[2] = { 63, 0 };
int Evaluator::blackEvalSquare[64];

Evaluator::Evaluator() {
	initBlackSquares();
}

Evaluator::Evaluator(Board* board, MovePrecomputation* precomputation) : precomputation(precomputation), board(board) {
	initBlackSquares();
}


void Evaluator::initVariables() {
	evaluation = 0;
	maximisingSide = board->state->whiteTurn ? 1 : -1;

	friendlyColor = board->state->whiteTurn ? Piece::white : Piece::black;
	enemyColor = !board->state->whiteTurn ? Piece::white : Piece::black;
	maximisingIndex = board->state->whiteTurn ? Board::whiteIndex : Board::blackIndex;
	minimisingIndex = !board->state->whiteTurn ? Board::whiteIndex : Board::blackIndex;

	friendlyKingSquare = board->pieceLists[maximisingIndex][Piece::king][0];
	enemyKingSquare = board->pieceLists[minimisingIndex][Piece::king][0];

	friendlyBoard = board->state->whiteTurn ? board->whitePieces : board->blackPieces;
	enemyBoard = !board->state->whiteTurn ? board->whitePieces : board->blackPieces;

	calculateEndgameWeight();

}

int Evaluator::Evaluate() {
	initVariables();

	evaluation += staticPieceEvaluations();
	evaluation += kingDist();

	return evaluation;
}

int Evaluator::kingTropism() {
	return 0;
}

int Evaluator::pawnShieldEvaluation() {
	return 0;
}

// Evaluates the passed pawns of either side
int Evaluator::passedPawnEvaluation() {
	int evaluation = 0;

	uint64_t opposingPawns;
	uint64_t passedMask;
	int color;
	int numPawns;
	int square;
	int promotionDistance;

	for (int colorIndex = Board::blackIndex; colorIndex <= Board::whiteIndex; colorIndex++) {
		opposingPawns = board->pawns & (colorIndex == maximisingIndex ? friendlyBoard : enemyBoard);
		color = colorIndex == Board::whiteIndex ? Piece::white : Piece::black;
		numPawns = board->pieceLists[colorIndex][Piece::pawn].numPieces;
		for (int i = 0; i < numPawns; i++) {
			square = board->pieceLists[colorIndex][Piece::pawn][i];
			passedMask = precomputation->getPassedPawnMask(square, color);

			// If no pawns in front of pawn's promotion path
			if ((passedMask & opposingPawns) == 0ULL) {
				promotionDistance = colorIndex == Board::whiteIndex ? square / 8 : 7 - (square / 8);
				if (colorIndex == maximisingIndex)
					evaluation += passedPawnBonus[promotionDistance];
				else
					evaluation -= passedPawnBonus[promotionDistance];
			}
		}
	}
	
	evaluation *= endGameWeight;

	return evaluation;
}

int Evaluator::pawnIslandEvaluation() {
	int evaluation = 0;

	uint64_t pawns;
	uint64_t islandMask;
	int col;
	int numPawns;
	int square;

	for (int colorIndex = Board::blackIndex; colorIndex <= Board::whiteIndex; colorIndex++) {
		pawns = board->pawns & (colorIndex == maximisingIndex ? friendlyBoard : enemyBoard);
		numPawns = board->pieceLists[colorIndex][Piece::pawn].numPieces;
		for (int i = 0; i < numPawns; i++) {
			square = board->pieceLists[maximisingIndex][Piece::pawn][i];
			islandMask = precomputation->getPawnIslandMask(square % 8);
			if ((pawns & islandMask) != 0ULL)
				continue;

			if (colorIndex == maximisingIndex)
				evaluation -= pawnIslandPenalty;
			else
				evaluation += pawnIslandPenalty;
		}
	}

	return evaluation;
}

void Evaluator::initBlackSquares() {
	for (int square = 0; square < 64; square++) {
		int row = square / 8;
		int col = square % 8;
		blackEvalSquare[square] = (7 - row) * 8 + col;
	}
}

int Evaluator::staticPieceEvaluations() {
	int evaluation;
	int* sideEval;
	int whiteEval = 0;
	int blackEval = 0;
	int numPieces;
	int pieceEval;
	int square;

	for (int colorIndex = Board::blackIndex; colorIndex <= Board::whiteIndex; colorIndex++) {
		sideEval = colorIndex == Board::blackIndex ? &blackEval : &whiteEval;
		for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
			numPieces = board->pieceLists[colorIndex][piece].numPieces;
			pieceEval = PieceEvaluations::pieceVals[piece];

			*sideEval += pieceEval * numPieces;

			for (int it = 0; it < numPieces; it++) {
				square = board->pieceLists[colorIndex][piece][it];
				if (colorIndex == Board::blackIndex) {
					square = blackEvalSquare[square];
				}
				switch (piece) {
				case Piece::pawn:
					*sideEval += (1 - endGameWeight) * (float) PieceEvaluations::pawnEval[square] + endGameWeight * (float) PieceEvaluations::pawnEndgameEval[square];
					break;
				case Piece::king:
					*sideEval += (1 - endGameWeight) * (float) PieceEvaluations::kingEval[square] + endGameWeight * (float) PieceEvaluations::kingEndgameEval[square];
					break;
				default:
					*sideEval += PieceEvaluations::pieceEvals[piece][square];
					break;
				}
			}
		}
	}

	evaluation = maximisingSide * (whiteEval - blackEval);

	return evaluation;
}

// Calculates mopup evaluation
int Evaluator::kingDist() {
	// If not an endgame or evaluation is too tight, dont bother with mopup evaluation
	if (endGameWeight == 0.f) {
		return 0;
	}

	int mopUpScore = 0;

	int losingKingSquare = evaluation > 0 ? enemyKingSquare : friendlyKingSquare;

	int losingKingCMD = arrCenterManhattanDistance[losingKingSquare];

	int kingsMD = abs((friendlyKingSquare / 8) - (enemyKingSquare / 8)) + abs((friendlyKingSquare % 8) - (enemyKingSquare % 8));

	// From Chess 4.x
	mopUpScore = 4.7f * losingKingCMD + 1.6f * (14 - kingsMD);
	mopUpScore *= endGameWeight;

	// Discourage moving to outer edge of board if losing
	if (evaluation < 0) {
		mopUpScore *= -1;
	}

	return mopUpScore;
}

void Evaluator::calculateEndgameWeight() {
	numMajorMinorPieces = 0;
	friendlyMajorMinorPieces = 0;
	enemyMajorMinorPieces = 0;

	for (int colorIndex = Board::blackIndex; colorIndex <= Board::whiteIndex; colorIndex++) {
		for (int piece = Piece::knight; piece < Piece::king; piece++) {
			if (colorIndex == maximisingIndex)
				friendlyMajorMinorPieces += board->pieceLists[colorIndex][piece].numPieces;
			else
				enemyMajorMinorPieces += board->pieceLists[colorIndex][piece].numPieces;
		}
	}

	if (friendlyMajorMinorPieces == 0 || enemyMajorMinorPieces == 0) {
		endGameWeight = 1;
		return;
	}

	numMajorMinorPieces = friendlyMajorMinorPieces + enemyMajorMinorPieces;

	// Square min component to create smoother curve 
	// (pow function is inherently slower than variable * variable)
	endGameWeight = min(1.f, numMajorMinorPieces / endgameRequiredPieces);
	endGameWeight *= endGameWeight;
	endGameWeight = 1.f - endGameWeight;
}
