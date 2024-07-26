#include "Evaluator.h"

Evaluator::Evaluator() {
}

int Evaluator::Evaluate(Board* board) {
	int evaluation = 0;
	int maximisingSide = board->state->whiteTurn ? Board::blackIndex : Board::whiteIndex;
	int multiplier;
	int direction;
	int startSquare;

	for (int colorIndex = Board::blackIndex; colorIndex <= Board::whiteIndex; colorIndex++) {
		multiplier = colorIndex == maximisingSide ? 1 : -1;
		direction = colorDirection[colorIndex];
		startSquare = colorStart[colorIndex];
		for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
			int numPieces = board->pieceLists[colorIndex][piece].numPieces;
			for (int it = 0; it < numPieces; it++) {
				int square = board->pieceLists[colorIndex][piece][it];
				evaluation += multiplier * (PieceEvaluations::pieceVals[piece] + PieceEvaluations::pieceEvals[piece][startSquare + direction * square]);
			}
		}
	}

	return evaluation;
}
