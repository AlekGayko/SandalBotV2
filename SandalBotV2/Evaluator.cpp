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

void Evaluator::initBlackSquares() {
	for (int square = 0; square < 64; square++) {
		int row = square / 8;
		int col = square % 8;
		blackEvalSquare[square] = (7 - row) * 8 + col;
	}
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
	evaluation += pawnIslandEvaluations();
	evaluation += passedPawnEvaluations();
	evaluation += pawnShieldEvaluations();
	evaluation += kingTropismEvaluations();

	return evaluation;
}

int Evaluator::kingTropismEvaluations() {
	int evaluation = kingTropism(friendlyKingSquare, board->pieceLists[maximisingIndex]);
	evaluation -= kingTropism(enemyKingSquare, board->pieceLists[minimisingIndex]);
	return evaluation;
}

int Evaluator::kingTropism(const int& kingSquare, PieceList* enemyList) {
	int evaluation = 0;
	int numPieces;
	int square;

	for (int piece = Piece::pawn; piece < Piece::king; piece++) {
		numPieces = enemyList[piece].numPieces;
		for (int it = 0; it < numPieces; it++) {
			square = enemyList[piece][it];

			evaluation -= (7 - precomputation->getDistance(kingSquare, square)) * tropismWeightings[piece];
		}
	}

	return evaluation;
}

int Evaluator::pawnShieldEvaluations() {
	if (endGameWeight >= 0.3f) {
		return 0;
	}

	uint64_t friendlyShield = precomputation->getShieldMask(friendlyKingSquare, friendlyColor);
	uint64_t enemyShield = precomputation->getShieldMask(enemyKingSquare, enemyColor);

	friendlyShield &= board->pawns & friendlyBoard;
	enemyShield &= board->pawns & enemyBoard;

	return (1 - endGameWeight) * (pawnShieldEvaluation(friendlyKingSquare, friendlyShield) - pawnShieldEvaluation(enemyKingSquare, enemyShield));
}

int Evaluator::pawnShieldEvaluation(const int& square, uint64_t& pawns) {
	int evaluation = 0;
	uint64_t attackMask;
	uint64_t pawnDefenders;
	uint64_t kingSquares = precomputation->getKingMoves(square);
	int targetSquare;
	int col = square % 8;

	// If there is no pawn shield on a column, add penalty
	if ((pawns & precomputation->getColMask(col)) == 0ULL) {
		evaluation -= pawnShieldColumnPenalty;
	}
	if (col < 7 && (pawns & precomputation->getColMask(col + 1)) == 0ULL) {
		evaluation -= pawnShieldColumnPenalty;
	}
	if (col > 0 && (pawns & precomputation->getColMask(col - 1)) == 0ULL) {
		evaluation -= pawnShieldColumnPenalty;
	}

	// Undefended pawns have penalty
	while (pawns != 0ULL) {
		targetSquare = BitBoardUtility::popLSB(pawns);
		attackMask = precomputation->getPawnAttackMoves(targetSquare, Piece::white);
		attackMask |= precomputation->getPawnAttackMoves(targetSquare, Piece::black);

		pawnDefenders = pawns & attackMask;

		if (pawnDefenders == 0ULL && (kingSquares & (1ULL << targetSquare)) == 0ULL) {
			evaluation -= pawnShieldUndefendedPenalty;
		}
	}

	return evaluation;
}

int Evaluator::passedPawnEvaluations() {
	uint64_t friendlyPawns = board->pawns & friendlyBoard;
	uint64_t enemyPawns = board->pawns & enemyBoard;

	int evaluation = passedPawnEvaluation(board->pieceLists[maximisingIndex][Piece::pawn], friendlyPawns, enemyPawns, friendlyColor);
	evaluation -= passedPawnEvaluation(board->pieceLists[minimisingIndex][Piece::pawn], enemyPawns, friendlyPawns, enemyColor);

	return evaluation;
}

// Evaluates the passed pawns of either side
int Evaluator::passedPawnEvaluation(PieceList& pieceList, uint64_t& pawns, uint64_t& opposingPawns, const int& color) {
	int evaluation = 0;

	uint64_t passedMask;
	int numPawns = pieceList.numPieces;
	int square;
	int promotionDistance;

	for (int i = 0; i < numPawns; i++) {
		square = pieceList[i];
		passedMask = precomputation->getPassedPawnMask(square, color);

		// If no pawns in front of pawn's promotion path
		if ((passedMask & opposingPawns) == 0ULL) {
			promotionDistance = color == Piece::white ? square / 8 : 7 - (square / 8);

			evaluation += passedPawnBonus[promotionDistance];
		}
	}

	return evaluation;
}

int Evaluator::pawnIslandEvaluations() {
	uint64_t friendlyPawns = board->pawns & friendlyBoard;
	uint64_t enemyPawns = board->pawns & enemyBoard;

	int evaluation = pawnIslandEvaluation(board->pieceLists[maximisingIndex][Piece::pawn], friendlyPawns);
	evaluation -= pawnIslandEvaluation(board->pieceLists[minimisingIndex][Piece::pawn], enemyPawns);

	return evaluation;
}

int Evaluator::pawnIslandEvaluation(PieceList& pieceList, uint64_t& pawns) {
	int evaluation = 0;

	uint64_t islandMask;
	int numPawns = pieceList.numPieces;
	int square;

	for (int i = 0; i < numPawns; i++) {
		square = pieceList[i];
		islandMask = precomputation->getPawnIslandMask(square % 8);
		if ((pawns & islandMask) != 0ULL)
			continue;

		evaluation -= pawnIslandPenalty;
	}

	return evaluation;
}

int Evaluator::staticPieceEvaluations() {
	int evaluation = staticPieceEvaluation(board->pieceLists[maximisingIndex]);
	evaluation -= staticPieceEvaluation(board->pieceLists[minimisingIndex], true);

	return evaluation;
}

int Evaluator::staticPieceEvaluation(PieceList* pieceList, bool useBlackSquares) {
	int evaluation = 0;
	int numPieces;
	int pieceEval;
	int square;

	for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
		numPieces = pieceList[piece].numPieces;
		pieceEval = PieceEvaluations::pieceVals[piece];

		evaluation += pieceEval * numPieces;

		for (int it = 0; it < numPieces; it++) {
			square = pieceList[piece][it];
			if (useBlackSquares)
				square = blackEvalSquare[square];

			switch (piece) {
			case Piece::pawn:
				evaluation += (1 - endGameWeight) * (float)PieceEvaluations::pawnEval[square] + endGameWeight * (float)PieceEvaluations::pawnEndgameEval[square];
				break;
			case Piece::king:
				evaluation += (1 - endGameWeight) * (float)PieceEvaluations::kingEval[square] + endGameWeight * (float)PieceEvaluations::kingEndgameEval[square];
				break;
			default:
				evaluation += PieceEvaluations::pieceEvals[piece][square];
				break;
			}
		}
	}

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
